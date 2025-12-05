#!/usr/bin/env python3
import socket
import struct
import sys

DWARF_IP = "192.168.8.223"
DWARF_WS_PORT = 9900

# Try simple socket connection first
print(f"Testing connection to {DWARF_IP}:{DWARF_WS_PORT}", flush=True)

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    result = sock.connect_ex((DWARF_IP, DWARF_WS_PORT))
    if result == 0:
        print(f"Port {DWARF_WS_PORT} is OPEN", flush=True)
    else:
        print(f"Port {DWARF_WS_PORT} is CLOSED (error {result})", flush=True)
    sock.close()
except Exception as e:
    print(f"Connection error: {e}", flush=True)

print("Done", flush=True)
