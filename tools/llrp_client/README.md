LLRP client tools

This folder contains a minimal Python-based LLRP testing client to connect to a reader via TCP, send basic LLRP messages and dump responses in hex. It is intentionally lightweight and dependency-free.

Usage:
  - Configure target host/port in environment variables or command args.
  - Run: python llrp_test_client.py --host 169.254.116.16 --port 5084

Notes:
  - This is a simple testing utility; for production use prefer the Java LLRP Toolkit (ltkjava) or a full LLRP implementation.
