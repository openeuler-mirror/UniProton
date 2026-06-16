#!/usr/bin/env python3
"""Host-side DTLS PSK client for the UniProton sd3403 mbedtls test."""

import argparse
import subprocess
import sys


DEFAULT_HOST = "192.168.7.2"
DEFAULT_PORT = 5658
DEFAULT_IDENTITY = "testserver1"
DEFAULT_PSK_ASCII = "11223344556677881122334455667788"
DEFAULT_MESSAGE = "Hi Server\n"
SERVER_REPLY = "Hi Client"


def run_client(args):
    psk_hex = args.psk_ascii.encode("ascii").hex()
    message = args.message
    if not args.no_newline and not message.endswith("\n"):
        message += "\n"

    cmd = [
        "openssl",
        "s_client",
        "-dtls1_2",
        "-connect",
        f"{args.host}:{args.port}",
        "-psk_identity",
        args.identity,
        "-psk",
        psk_hex,
        "-quiet",
    ]

    print("Running:", " ".join(cmd), file=sys.stderr)
    print(f"Sending: {message!r}", file=sys.stderr)

    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    try:
        output, _ = proc.communicate(message, timeout=args.timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        output, _ = proc.communicate()
        print(f"Timed out after {args.timeout}s", file=sys.stderr)
        print(output, end="")
        return 124

    lines = output.splitlines()
    if not args.show_openssl_warnings:
        lines = [line for line in lines if line != "Can't use SSL_get_servername"]

    filtered_output = "\n".join(lines).strip()
    if filtered_output:
        print(f"Received: {filtered_output}")
    else:
        print("Received: <empty>")

    if SERVER_REPLY in output:
        print("DTLS client test passed", file=sys.stderr)
    else:
        print("DTLS client did not receive server reply", file=sys.stderr)
    print(f"OpenSSL client exit code: {proc.returncode}", file=sys.stderr)
    if SERVER_REPLY not in output and proc.returncode == 0:
        return 1
    return proc.returncode


def parse_args():
    parser = argparse.ArgumentParser(
        description="Send one DTLS PSK message to the UniProton sd3403 mbedtls server."
    )
    parser.add_argument("--host", default=DEFAULT_HOST, help="server host/IP")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help="server UDP port")
    parser.add_argument("--identity", default=DEFAULT_IDENTITY, help="PSK identity")
    parser.add_argument(
        "--psk-ascii",
        default=DEFAULT_PSK_ASCII,
        help="ASCII PSK string; converted to hex for openssl -psk",
    )
    parser.add_argument("--message", default=DEFAULT_MESSAGE, help="message to send")
    parser.add_argument(
        "--no-newline",
        action="store_true",
        help="do not append a newline when --message has none",
    )
    parser.add_argument("--timeout", type=int, default=20, help="client timeout seconds")
    parser.add_argument(
        "--show-openssl-warnings",
        action="store_true",
        help="show benign openssl s_client warning lines such as missing SNI",
    )
    return parser.parse_args()


if __name__ == "__main__":
    sys.exit(run_client(parse_args()))
