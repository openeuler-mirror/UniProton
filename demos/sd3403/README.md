# SD3403 OpenArm dual-arm control on UniProton

Run OpenArm dual-arm drag-and-hold control on the SD3403 (SS928) board.
The `openarm` app runs the dual-arm controller and the CAN-FD gateway on
UniProton @ CPU3 (Cortex-A55); Linux starts and stops the core through
MICA and provides the control tools, but is not in the control data path.

```
 Linux (A55 x3)               UniProton @ CPU3                  OpenArm
 MICA lifecycle    <--MICA-->  control task  <--Direct IPC-->  gateway task
 direct-ctl        <--/dev/mem--> channels                       | SPI0  right arm
                                                                 | SPI2  left arm
                                                                 v
                                                  2x MCP2518FD ---> 7 joints + gripper
```

The gateway task beats at 1 kHz: it sends the frames queued in the previous
beat on both buses, collects the replies, runs one state-machine step per
arm and publishes feedback, then wakes the control task, which computes the
next targets for both arms.

## Board support

- `bsp/sd3403/cpu_config.h`, `build/sd3403.ld`: image at 0x4b000000, as
  in the board vendor's firmware; OpenAMP shared memory at 0x48000000,
  followed by the log memory and the WOW region, inside the board's
  `mcs_baremetal_mem` reservation (0x48000000-0x4affffff).
- `apps/openamp/main.c`: the wait for the tty endpoint at startup is
  bounded to 5 s, so the application also starts when nobody opens the
  console.
- `bsp/timer.c`, `config/prt_config.h`: 1 kHz OS tick on an absolute
  deadline grid; the counter runs at 24 MHz (`SYS_COUNTER_FREQ`), not the
  25 MHz the firmware leaves in CNTFRQ_EL0. Tickless idle
  (`src/extended/lowpower/tickless/prt_tickless.c`) still takes its reload
  from CNTFRQ_EL0, so the tick runs about 4% slow whenever it sleeps; the
  sd3403 defconfig leaves tickless off.

## Components

| Path | What it is |
|---|---|
| `apps/openamp/direct_ipc.{c,h}` | Per-arm Direct IPC channels: SPSC target/feedback rings, CRC, generation handshake, fault bits and Linux commands; the dual-arm control task; MICA CPU_OFF handling |
| `apps/openamp/openarm_control_dual.h`, `openarm_control_lib_dual.cc` | Two independent 7-joint admittance controllers with virtual clutches, pinocchio gravity and friction feedforward, plus the gripper clutch |
| `apps/openamp/canfd_gw_{spi,can,gateway}.{c,h}` | 1 kHz gateway: PL022 PIO SPI, two MCP2518FDs and the per-arm state machines |
| `apps/openamp/openarm_can.{c,h}` | Damiao CAN frame encoding, decoding and command clamping |
| `apps/openamp/openarm_gravity.{c,h}` | Analytic gravity reference used by the algorithm-library tests |
| `tools/direct-ctl.py` | Board-side status, arm/disarm, fault reset, mask, parameter and CAN bus configuration control |
| `tools/openarm.ini` | Parameter sample; all eight motors (seven joints and the gripper) enabled |

## Build

In the UniProton v004 build container:

```bash
cd demos/sd3403/build
bash build_app.sh sd3403        # default image: board support only
bash build_app.sh openarm       # dual-arm controller + CAN-FD gateway
```

`openarm` needs `CONFIG_OS_SUPPORT_EIGEN=y` and
`CONFIG_OS_SUPPORT_PINOCCHIO=y` in
`build/uniproton_config/config_armv8_sd3403/defconfig`. The C++ build
downloads the enabled library sources into `component/` and builds the
bare-metal libstdc++ on first use; those generated dependencies are not
committed.

## Deploy

Strip `openarm.elf` and install it at the ClientPath of the `uniproton`
MICA instance (`/root/mica/sd3403.elf` on the board), copy
`tools/direct-ctl.py` and `tools/openarm.ini` to `/root/mica/`, then
start the instance. By default SPI0 drives the right arm and SPI2 the
left arm; see `set-can` below if the arm cables are the other way round.
Linux must configure the SPI pinmux before start, with a pad drive
strength of at least 3 on the SPI outputs: otherwise the gateway's 12.5
MHz check fails and the buses fall back to 6.25 MHz (bits 2/3 of the
statistics flags word, `canfd_gw_gateway.h`).

## Runtime control

```bash
python3 /root/mica/direct-ctl.py status
python3 /root/mica/direct-ctl.py apply-ini /root/mica/openarm.ini
python3 /root/mica/direct-ctl.py set-mask 0x7f   # joints only, gripper out of the loop
python3 /root/mica/direct-ctl.py set-can swapped # right arm on SPI2, left arm on SPI0
python3 /root/mica/direct-ctl.py set-tdco 6      # CAN FD TDC offset of both buses
python3 /root/mica/direct-ctl.py disarm
python3 /root/mica/direct-ctl.py reset-faults   # clear faults, new session, default parameters
```

Every command applies to both arms, which always share one mask. The
firmware powers up with mask 0xff (all seven joints and the gripper), the
built-in joint parameters, the default bus mapping and TDC offset (TDCO)
6, the 5 Mbit/s data-phase sample point in 25 ns units. The sample
`openarm.ini` keeps these defaults except for the J1 inertia damping (0.2)
and capture (0.05). apply-ini checks the whole file, and that both arms
are SAFE if the CAN settings change, before writing anything; then it
sets the mask and parameters, and the CAN settings last.
reset-faults starts a new session with the default parameters (masks and
CAN settings are kept), and a reboot restores every default; run
apply-ini again after either.

The mask and the parameters are taken only while an arm is DISARMED or in
fault. A bus swap or TDCO change is taken only while no motor of either
arm is enabled or awaiting a confirmed disable and no arm request is
pending; each disarmed arm then rediscovers its motors, and an arm in
fault stays there until reset-faults. If 5 Mbit/s frames fail with
data-phase bit errors on a given harness, TDCO can be adjusted without
rebuilding. An arm reaches DISARMED only once every motor in its mask
replies, and faults if one stays silent; set-mask is also accepted in
that fault, so clear the missing motor's bit (bit 7 for the gripper) and
run reset-faults to rediscover.

Arming requires both arms DISARMED and SAFE without faults, and an
explicit operator confirmation token; see `direct-ctl.py arm --help`.
Disable completion is confirmed by motor replies: DISABLE is retried for
motors still reporting enabled, an arm reports SAFE only once no motor is
enabled and no disable is pending, and a new arm request is held until
then. `mica stop` sends DISABLE to every motor before the core powers
off.

Each joint reference stays inside the URDF joint limits. Past a limit
the joint spring is kp times the overshoot and is not capped by the
yield torque, so a joint armed outside its limit is pulled back at
once: about 2 Nm for J4 0.08 rad below its 0 rad limit with the default
kp of 26 Nm/rad. Arm with every joint inside its limits.

## Message protocol (little-endian)

Each arm has an independent channel (right at `0x40100000`, left at
`+8 KiB`) with 16-entry target and feedback rings. Every message covers
eight motors: seven joints and the gripper in slot 7 (CAN 0x08/0x18); bit
7 of a joint mask selects the gripper.

| Message | Size |
|---|---:|
| target (control task -> gateway): header, qTarget/dqTarget/tauFf/kp/kd[8], crc32 | 184 B |
| feedback (gateway -> control task): header, q/dq/tau[8], temperatures, online/valid masks, crc32 | 148 B |

Gripper telemetry sits in a sequence-locked block at channel `+0x1800`,
and the right channel carries the CAN configuration request/acknowledge
block (bus mapping and TDCO) at `+0x1840`. Gripper commands are sent
every four gateway beats. Both sides verify magic, version and CRC, and
a corrupt message raises a fault bit instead of being acted on. A
generation bump resets the session on both ends; messages of an earlier
generation still in the rings are dropped without a fault. The
gateway publishes its beat statistics at `0x4010F000`
(`canfd_gw_gateway.h`).
