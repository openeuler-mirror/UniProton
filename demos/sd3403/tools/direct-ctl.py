#!/usr/bin/env python3
"""Inspect and control the openarm Direct IPC channels.

Every operation applies to both channels (right arm 0x40100000, left arm
0x40102000) and the status shows both. Each arm carries seven joints and
the gripper as motor 8: bit 7 of the joint mask, INI key [joints]
gripper.

The two arms can also swap CAN buses (set-can), and set-tdco sets the
transmitter delay compensation offset of both buses; both are also read
from INI [can]. Every boot restores the defaults: right arm on SPI0, left
arm on SPI2, TDCO 6. A change needs every motor disabled and no arm
request; the arms then rediscover their motors, except an arm in FAULT,
which stays there until reset-faults.

reset-faults also restores the default joint parameters (the masks and
the CAN settings are kept); run apply-ini again afterwards.
"""

import argparse
import configparser
import math
import mmap
import os
import struct
import time


CHANNEL_ADDRESSES = (0x40100000, 0x40102000)   # right arm, left arm
CHANNEL_SIZE = 0x2000
CHANNEL_MAGIC = 0x52445452
CHANNEL_VERSION = 1
CHANNEL_HEADER_SIZE = 364

TEST_ARM = 0x54450006
TEST_DISARM = 0x54450007
TEST_RESET_FAULTS = 0x5445FFFF

# direct_ipc.h struct DirectIpcChannel: header words up to configAckSequence
HEADER_WORDS = 42
STATUS_PRODUCER_WORD = 16
TEST_COMMAND_OFFSET = 132
TEST_ARGUMENT_OFFSET = 136
TEST_SEQUENCE_OFFSET = 140
TEST_ACK_OFFSET = 144
TEST_RESULT_OFFSET = 148
REQUESTED_MASK_OFFSET = 152
EFFECTIVE_MASK_OFFSET = 156
CONFIG_SEQUENCE_OFFSET = 160
CONFIG_ACK_OFFSET = 164
PARAM_OFFSET = 168
PARAM_NAMES = ("gravity_scale", "gravity_offset", "kp", "kd", "yield_torque",
               "inertia_damping", "inertia_capture")
PARAM_OFFSETS = {name: PARAM_OFFSET + index * 28 for index, name in enumerate(PARAM_NAMES)}
PARAM_SCALE = 1000000
# DirectJointParamsValid, in PARAM_SCALE units: (low, high, low excluded)
PARAM_LIMITS = {
    "gravity_scale": (0, 3000000, False),
    "gravity_offset": (-20000000, 20000000, False),
    "kp": (0, 100000000, True),
    "kd": (0, 5000000, False),
    "yield_torque": (0, 10000000, True),
    "inertia_damping": (0, 10000000, False),
    "inertia_capture": (0, 2000000, False),
}

# feedback ring: struct DirectRobotFeedbackMessage after the 16 targets
RING_CAP = 16
TARGET_SIZE = 184
FEEDBACK_SIZE = 148
FEEDBACK_RING_OFFSET = CHANNEL_HEADER_SIZE + RING_CAP * TARGET_SIZE
FEEDBACK_MAGIC = 0x4F41524D
FEEDBACK_GENERATION_OFFSET = 16
FEEDBACK_Q_OFFSET = 28

STATE_NAMES = ("CTRL_READY", "GW_READY", "RUNNING", "FAULT", "SAFE", "RECOVERING",
               "STOPPING", "ARMED", "DISARMED", "DISCOVERED")
STATE_FAULT = 1 << 3
STATE_SAFE = 1 << 4
STATE_ARMED = 1 << 7
STATE_DISARMED = 1 << 8
# the gateway takes a new mask or parameters while disarmed or in fault
STATE_CONFIGURABLE = STATE_DISARMED | STATE_FAULT
ALL_JOINTS_MASK = 0xFF
GRIPPER_BIT = 1 << 7

ARM_CONFIRMATION = "SUPPORTED-ARM-ESTOP-READY"

# direct_ipc.h DirectCanConfig: right-arm channel +0x1840; words magic,
# map, tdco, sequence, ackSequence, result, appliedMap, appliedTdco, changes
CAN_CFG_ADDRESS = CHANNEL_ADDRESSES[0] + 0x1000
CAN_CFG_MAP_SIZE = 0x1000
CAN_CFG_OFFSET = 0x840
CAN_CFG_MAGIC = 0x314E4143
CAN_MAPS = {"default": 0, "swapped": 1}
CAN_MAP_BUSES = {0: ("SPI0", "SPI2"), 1: ("SPI2", "SPI0")}   # right, left
CAN_TDCO_DEFAULT = 6
CAN_TDCO_MAX = 63
CAN_CFG_ACCEPTED = 1
CAN_CFG_BUSY = 2

INI_KEYS = {
    "arm": {"active_joint_mask"},
    "can": {"right", "left", "tdco"},
    "joints": {f"j{index + 1}" for index in range(7)} | {"gripper"},
    "gravity": {f"j{index + 1}_{name}" for index in range(7)
                for name in ("gravity_scale", "gravity_offset")},
    "impedance": {f"j{index + 1}_{name}" for index in range(7) for name in ("kp", "kd")},
    "clutch": {f"j{index + 1}_yield_torque" for index in range(7)},
    "inertia": {f"j{index + 1}_{name}" for index in range(7)
                for name in ("inertia_damping", "inertia_capture")},
}
INI_PARAM_SECTIONS = {
    "gravity": ("gravity_scale", "gravity_offset"),
    "impedance": ("kp", "kd"),
    "clutch": ("yield_torque",),
    "inertia": ("inertia_damping", "inertia_capture"),
}


def parse_int(text):
    """0x.. hex or decimal; a leading zero is decimal (tdco = 06)."""
    text = text.strip()
    try:
        return int(text, 0)
    except ValueError:
        return int(text, 10)


def open_channels():
    device = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
    mappings = []
    try:
        for address in CHANNEL_ADDRESSES:
            mappings.append(mmap.mmap(
                device,
                CHANNEL_SIZE,
                flags=mmap.MAP_SHARED,
                prot=mmap.PROT_READ | mmap.PROT_WRITE,
                offset=address,
            ))
    except Exception:
        for mapping in mappings:
            mapping.close()
        os.close(device)
        raise
    return device, mappings


def open_can_config(device):
    # The gateway publishes the block once both CAN controllers are set up.
    mapping = mmap.mmap(
        device,
        CAN_CFG_MAP_SIZE,
        flags=mmap.MAP_SHARED,
        prot=mmap.PROT_READ | mmap.PROT_WRITE,
        offset=CAN_CFG_ADDRESS,
    )
    if struct.unpack_from("<I", mapping, CAN_CFG_OFFSET)[0] != CAN_CFG_MAGIC:
        mapping.close()
        return None
    return mapping


def read_words(mapping):
    return struct.unpack_from(f"<{HEADER_WORDS}I", mapping, 0)


def verify_channel(words):
    if (words[0] != CHANNEL_MAGIC or words[1] != CHANNEL_VERSION
            or words[2] != CHANNEL_HEADER_SIZE):
        raise RuntimeError(
            f"direct channel unavailable: magic=0x{words[0]:08x} version={words[1]} "
            f"header={words[2]}"
        )


def state_names(state):
    return "|".join(name for bit, name in enumerate(STATE_NAMES) if state & (1 << bit)) or "-"


def latest_positions(mapping, words):
    """q of the newest feedback of this generation, or None."""
    slot = (words[STATUS_PRODUCER_WORD] - 1) % RING_CAP
    base = FEEDBACK_RING_OFFSET + slot * FEEDBACK_SIZE
    magic = struct.unpack_from("<I", mapping, base)[0]
    generation = struct.unpack_from("<I", mapping, base + FEEDBACK_GENERATION_OFFSET)[0]
    if magic != FEEDBACK_MAGIC or generation != words[3]:
        return None
    return struct.unpack_from("<8f", mapping, base + FEEDBACK_Q_OFFSET)


def print_status(mappings):
    for index, mapping in enumerate(mappings):
        label = "R" if index == 0 else "L"
        words = read_words(mapping)
        verify_channel(words)
        print(f"--- channel {label} @ {CHANNEL_ADDRESSES[index]:#x} version {words[1]}")
        print(
            f"generation={words[3]} state=0x{words[4]:08x} "
            f"ctrl_state=0x{words[5]:08x} gw_state=0x{words[6]:08x} "
            f"({state_names(words[6])})"
        )
        print(
            f"heartbeat ctrl={words[9]} gw={words[10]} "
            f"stage ctrl={words[24]} gw={words[25]}"
        )
        print(
            f"fault active=0x{words[29]:08x} ctrl=0x{words[30]:08x} "
            f"gw=0x{words[31]:08x} latched=0x{words[32]:08x}"
        )
        print(
            f"target_sequence={words[22]} feedback_sequence={words[23]} "
            f"target_overflow={words[18]} feedback_overflow={words[19]}"
        )
        gripper = "in" if words[39] & GRIPPER_BIT else "out"
        print(
            f"joint_mask requested=0x{words[38]:02x} effective=0x{words[39]:02x} "
            f"gripper={gripper} config_sequence={words[40]} config_ack={words[41]}"
        )
        q = latest_positions(mapping, words)
        if q is None:
            print("q unavailable (no feedback in this generation)")
        else:
            print("q J1-J7 " + " ".join(f"{value:+.3f}" for value in q[:7])
                  + f" gripper {q[7]:+.3f} rad")


def read_can_config(can_config):
    """Return (applied map, applied TDCO, changes)."""
    return struct.unpack_from("<III", can_config, CAN_CFG_OFFSET + 24)


def print_can_config(can_config):
    applied, tdco, changes = read_can_config(can_config)
    right, left = CAN_MAP_BUSES.get(applied, ("?", "?"))
    print(f"--- CAN: right arm on {right}, left arm on {left}, TDCO {tdco} "
          f"(changes={changes})")


def scaled(name, joint, value):
    if not math.isfinite(value):
        raise ValueError(f"j{joint + 1}_{name} = {value} is not a number")
    low, high, low_excluded = PARAM_LIMITS[name]
    result = int(round(value * PARAM_SCALE))
    if result < low or result > high or (low_excluded and result == low):
        bound = "(" if low_excluded else "["
        raise ValueError(f"j{joint + 1}_{name} = {value} is outside "
                         f"{bound}{low / PARAM_SCALE:g}, {high / PARAM_SCALE:g}]")
    return result


def configure(mappings, joint_mask=None, values=None):
    """Write the mask and/or parameter rows, then one sequence per channel."""
    if joint_mask is not None and (joint_mask == 0 or (joint_mask & ~ALL_JOINTS_MASK) != 0):
        raise ValueError("joint mask must be 0x01-0xff: bits 0-6 = J1-J7, bit 7 = gripper")
    rows = {name: [scaled(name, joint, value) for joint, value in enumerate(row)]
            for name, row in (values or {}).items()}
    for mapping in mappings:
        words = read_words(mapping)
        verify_channel(words)
        if (words[6] & STATE_CONFIGURABLE) == 0:
            raise RuntimeError("the joint mask and parameters can only be changed while "
                               "DISARMED or in FAULT")
    for mapping in mappings:
        for name, row in rows.items():
            for index, value in enumerate(row):
                struct.pack_into("<i", mapping, PARAM_OFFSETS[name] + index * 4, value)
        if joint_mask is not None:
            struct.pack_into("<I", mapping, REQUESTED_MASK_OFFSET, joint_mask)
        sequence = (struct.unpack_from("<I", mapping, CONFIG_SEQUENCE_OFFSET)[0] + 1) \
            & 0xFFFFFFFF
        struct.pack_into("<I", mapping, CONFIG_SEQUENCE_OFFSET, sequence)
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            effective = struct.unpack_from("<I", mapping, EFFECTIVE_MASK_OFFSET)[0]
            ack = struct.unpack_from("<I", mapping, CONFIG_ACK_OFFSET)[0]
            if ack == sequence and (joint_mask is None or effective == joint_mask):
                break
            time.sleep(0.01)
        else:
            raise TimeoutError("the gateway did not acknowledge the joint mask or parameters")


def wait_disarmed(mappings, faulted, timeout):
    """Wait for every arm that was not in FAULT to rediscover its motors."""
    # The acknowledge and the fall back to discovery happen in one gateway
    # beat; give that beat time to finish before looking for DISARMED.
    time.sleep(0.02)
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        states = [read_words(mapping)[6] for index, mapping in enumerate(mappings)
                  if not faulted[index]]
        if all(state & STATE_DISARMED for state in states):
            return
        if any(state & STATE_FAULT for state in states):
            break
        time.sleep(0.02)
    raise RuntimeError(
        "an arm did not rediscover its motors after the CAN change; check "
        "that arm, its cable and the TDCO, then run reset-faults")


def can_request(can_config, can_map=None, tdco=None):
    """The (bus mapping, TDCO) to request, or None if nothing changes."""
    if can_config is None:
        if can_map in (None, CAN_MAPS["default"]) and tdco is None:
            return None
        raise RuntimeError("the gateway has not published its CAN configuration yet")
    if tdco is not None and not 0 <= tdco <= CAN_TDCO_MAX:
        raise ValueError(f"TDCO must be 0..{CAN_TDCO_MAX}")
    applied_map, applied_tdco, _ = read_can_config(can_config)
    request = (applied_map if can_map is None else can_map,
               applied_tdco if tdco is None else tdco)
    return None if request == (applied_map, applied_tdco) else request


def set_can_config(mappings, can_config, can_map=None, tdco=None):
    """Request a bus mapping and/or TDCO; None keeps the applied value."""
    request = can_request(can_config, can_map, tdco)
    if request is None:
        return
    can_map, tdco = request
    faulted = [bool(read_words(mapping)[6] & STATE_FAULT) for mapping in mappings]
    sequence = (struct.unpack_from("<I", can_config, CAN_CFG_OFFSET + 12)[0] + 1) \
        & 0xFFFFFFFF
    # One word per call: pack_into zero-fills its whole span first, and on
    # this Device mapping an 8-byte fill at a 4-byte boundary faults.
    struct.pack_into("<I", can_config, CAN_CFG_OFFSET + 4, can_map)
    struct.pack_into("<I", can_config, CAN_CFG_OFFSET + 8, tdco)
    struct.pack_into("<I", can_config, CAN_CFG_OFFSET + 12, sequence)
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        ack, result = struct.unpack_from("<II", can_config, CAN_CFG_OFFSET + 16)
        if ack == sequence:
            break
        time.sleep(0.01)
    else:
        raise TimeoutError("the gateway did not acknowledge the CAN configuration")
    if result == CAN_CFG_BUSY:
        raise RuntimeError("CAN configuration refused: disarm both arms and wait "
                           "until every motor reports disabled")
    if result != CAN_CFG_ACCEPTED:
        raise RuntimeError(f"CAN configuration rejected: result={result}")
    wait_disarmed(mappings, faulted, 3.0)
    if any(faulted):
        print("CAN configuration applied; run reset-faults to rediscover the arm in FAULT")


def read_ini(path):
    config = configparser.ConfigParser()
    if not config.read(path):
        raise FileNotFoundError(path)
    if config.defaults():
        raise ValueError("INI [DEFAULT] is not used; move its keys to their sections")
    for section in config.sections():
        if section not in INI_KEYS:
            raise ValueError(f"INI section [{section}] is unknown")
        unknown = sorted(set(config.options(section)) - INI_KEYS[section])
        if unknown:
            raise ValueError(f"INI [{section}] has unknown keys: {', '.join(unknown)}")
    return config


def can_config_from_ini(config):
    """Return (bus mapping or None, TDCO or None) from [can]."""
    if not config.has_section("can"):
        return None, None
    can_map = None
    if config.has_option("can", "right") or config.has_option("can", "left"):
        buses = (config.get("can", "right", fallback="").strip().upper(),
                 config.get("can", "left", fallback="").strip().upper())
        for request, pair in CAN_MAP_BUSES.items():
            if buses == pair:
                can_map = request
        if can_map is None:
            raise ValueError("[can] needs right and left, one on spi0 and one on spi2")
    tdco = None
    if config.has_option("can", "tdco"):
        tdco = parse_int(config.get("can", "tdco"))
        if not 0 <= tdco <= CAN_TDCO_MAX:
            raise ValueError(f"[can] tdco must be 0..{CAN_TDCO_MAX}")
    return can_map, tdco


def mask_from_ini(config):
    configured = None
    if config.has_option("arm", "active_joint_mask"):
        configured = parse_int(config.get("arm", "active_joint_mask"))
    if config.has_section("joints"):
        missing = sorted(INI_KEYS["joints"] - set(config.options("joints")))
        if missing:
            raise ValueError(f"INI [joints] must list every motor; missing: {', '.join(missing)}")
        joint_mask = 0
        for index in range(7):
            if config.getboolean("joints", f"j{index + 1}"):
                joint_mask |= 1 << index
        if config.getboolean("joints", "gripper"):
            joint_mask |= GRIPPER_BIT
        if configured is not None and joint_mask != configured:
            raise ValueError("[arm] active_joint_mask disagrees with [joints]")
        configured = joint_mask
    if configured is None:
        raise ValueError("INI must define active_joint_mask or [joints]")
    return configured


def parameters_from_ini(config, mapping):
    """Rows to write: the current row of every parameter the INI sets, updated."""
    values = {}
    current = {}
    for name in PARAM_NAMES:
        current[name] = [
            struct.unpack_from("<i", mapping, PARAM_OFFSETS[name] + index * 4)[0]
            / PARAM_SCALE
            for index in range(7)
        ]
    for section, names in INI_PARAM_SECTIONS.items():
        for name in names:
            row = current[name][:]
            found = False
            for index in range(7):
                key = f"j{index + 1}_{name}"
                if config.has_option(section, key):
                    row[index] = config.getfloat(section, key)
                    found = True
            if found:
                values[name] = row
    return values


def send_command(mappings, command, argument=0):
    """Post a command once: the control task applies it to both arms."""
    for mapping in mappings:
        verify_channel(read_words(mapping))
    mapping = mappings[0]
    sequence = (read_words(mapping)[35] + 1) & 0xFFFFFFFF
    # One word per call, as in set_can_config: testCommand sits at a
    # 4-byte boundary, where an 8-byte fill faults on this mapping.
    struct.pack_into("<I", mapping, TEST_COMMAND_OFFSET, command)
    struct.pack_into("<I", mapping, TEST_ARGUMENT_OFFSET, argument)
    struct.pack_into("<I", mapping, TEST_SEQUENCE_OFFSET, sequence)
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        ack, result = struct.unpack_from("<II", mapping, TEST_ACK_OFFSET)
        if ack == sequence:
            if result != 1:
                raise RuntimeError(f"command rejected: result={result}")
            return
        time.sleep(0.01)
    raise TimeoutError("the control task did not acknowledge the command")


def arm_both(mappings):
    """Arm only from DISARMED and SAFE without faults, and confirm both arm."""
    for index, mapping in enumerate(mappings):
        words = read_words(mapping)
        verify_channel(words)
        state = words[6]
        if (not state & STATE_DISARMED or not state & STATE_SAFE or state & STATE_FAULT
                or words[29] != 0):
            label = "R" if index == 0 else "L"
            raise RuntimeError(
                f"arm {label} is not ready: gw_state {state_names(state)}, faults "
                f"0x{words[29]:08x}; both arms must be DISARMED|SAFE without faults")
    send_command(mappings, TEST_ARM)
    deadline = time.monotonic() + 1.0
    while time.monotonic() < deadline:
        if all(read_words(mapping)[6] & STATE_ARMED for mapping in mappings):
            return
        time.sleep(0.01)
    send_command(mappings, TEST_DISARM)
    raise RuntimeError("an arm did not arm within 1 s; disarm sent to both arms")


def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("status")
    arm = subparsers.add_parser("arm", help="enable both arms (operator at the e-stop)")
    arm.add_argument("--confirm", required=True,
                     help=f"must be {ARM_CONFIRMATION}: the e-stop is within reach")
    subparsers.add_parser("disarm")
    subparsers.add_parser(
        "reset-faults", help="clear faults and restart both sessions; restores the "
        "default joint parameters")
    set_mask = subparsers.add_parser("set-mask")
    set_mask.add_argument("mask", type=parse_int)
    apply_ini = subparsers.add_parser("apply-ini")
    apply_ini.add_argument("path")
    set_can = subparsers.add_parser(
        "set-can", help="default: right arm SPI0, left arm SPI2; swapped: the reverse")
    set_can.add_argument("map", choices=sorted(CAN_MAPS))
    set_tdco = subparsers.add_parser(
        "set-tdco", help="CAN FD transmitter delay compensation offset of both buses, "
        f"0..{CAN_TDCO_MAX} x 25 ns (default {CAN_TDCO_DEFAULT})")
    set_tdco.add_argument("tdco", type=parse_int)
    args = parser.parse_args()

    if args.command == "arm" and args.confirm != ARM_CONFIRMATION:
        parser.error(f"arm requires --confirm {ARM_CONFIRMATION}")

    device, mappings = open_channels()
    can_config = None
    try:
        can_config = open_can_config(device)
        if args.command == "set-mask":
            configure(mappings, joint_mask=args.mask)
        elif args.command == "set-can":
            set_can_config(mappings, can_config, can_map=CAN_MAPS[args.map])
        elif args.command == "set-tdco":
            set_can_config(mappings, can_config, tdco=args.tdco)
        elif args.command == "apply-ini":
            config = read_ini(args.path)
            can_map, tdco = can_config_from_ini(config)
            joint_mask = mask_from_ini(config)
            parameters = parameters_from_ini(config, mappings[0])
            if can_config is None and tdco == CAN_TDCO_DEFAULT:
                tdco = None
            # check what the CAN step needs before anything is written
            if can_request(can_config, can_map, tdco) is not None and not all(
                    read_words(mapping)[6] & STATE_SAFE for mapping in mappings):
                raise RuntimeError("CAN configuration refused: disarm both arms and wait "
                                   "until every motor reports disabled")
            configure(mappings, joint_mask, parameters)
            set_can_config(mappings, can_config, can_map, tdco)
        elif args.command == "arm":
            arm_both(mappings)
        elif args.command == "disarm":
            send_command(mappings, TEST_DISARM)
        elif args.command == "reset-faults":
            send_command(mappings, TEST_RESET_FAULTS)
            print("joint parameters are back to the defaults; run apply-ini to set them again")
        print_status(mappings)
        if can_config is not None:
            print_can_config(can_config)
    finally:
        if can_config is not None:
            can_config.close()
        for mapping in mappings:
            mapping.close()
        os.close(device)


if __name__ == "__main__":
    main()
