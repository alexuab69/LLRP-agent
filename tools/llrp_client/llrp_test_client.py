#!/usr/bin/env python3
"""
Minimal LLRP test client (Python)
- Connects to a reader via TCP
- Sends ADD_ROSPEC / ENABLE_ROSPEC / START_ROSPEC minimal messages (header-only or small payload)
- Dumps received bytes in hex

Usage: python llrp_test_client.py --host 169.254.116.16 --port 5084
"""
import argparse
import socket
import struct
import time
from config import DEFAULT_HOST, DEFAULT_PORT


def build_header(version, msg_type, message_id, length=10):
    # First 2 bytes: version (3 bits at bits 2-4) and type (11 bits)
    header_field = (version << 13) | (msg_type & 0x7FF)
    return struct.pack('>HII', header_field & 0xFFFF, length, message_id)


def hexdump(b):
    return ' '.join(f'{x:02X}' for x in b)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', default=None)
    parser.add_argument('--port', type=int, default=None)
    parser.add_argument('--timeout', type=float, default=5.0)
    args = parser.parse_args()

    host = args.host or DEFAULT_HOST
    port = args.port or DEFAULT_PORT
    print(f"Using reader host={host} port={port}")

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5)
    s.connect((host, port))
    s.settimeout(args.timeout)
    print(f'Connected to {host}:{port}')

    try:
        # ADD_ROSPEC minimal
        add_msg = build_header(1, 20, 1, length=10+8)
        add_msg += b'\x00\x00\x00\x00\x00\x00\x00\x00'  # small payload
        s.sendall(add_msg)
        print('Sent ADD_ROSPEC (minimal)')

        # Read any response
        try:
            data = s.recv(4096)
            if data:
                print('Recv:', hexdump(data))
        except socket.timeout:
            pass

        # ENABLE_ROSPEC (header-only)
        enable_msg = build_header(1, 24, 2)
        s.sendall(enable_msg)
        print('Sent ENABLE_ROSPEC (header-only)')

        try:
            data = s.recv(4096)
            if data:
                print('Recv:', hexdump(data))
        except socket.timeout:
            pass

        # START_ROSPEC
        start_msg = build_header(1, 22, 3)
        s.sendall(start_msg)
        print('Sent START_ROSPEC (header-only)')

        # Read for some time
        end = time.time() + 10
        while time.time() < end:
            try:
                data = s.recv(8192)
                if data:
                    print('Recv:', hexdump(data))
            except socket.timeout:
                continue

        # STOP_ROSPEC
        stop_msg = build_header(1, 23, 4)
        s.sendall(stop_msg)
        print('Sent STOP_ROSPEC (header-only)')

    finally:
        s.close()
        print('Disconnected')


if __name__ == '__main__':
    main()
