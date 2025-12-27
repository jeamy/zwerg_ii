#!/usr/bin/env python3
import sys

filename = "ctrl_20251226_124424.pcapng"
print(f"Checking {filename}...", file=sys.stderr)

try:
    with open(filename, 'rb') as f:
        magic = f.read(4)
        print(f"Magic bytes: {magic.hex()}", file=sys.stderr)
        
        size = f.seek(0, 2)
        print(f"File size: {size} bytes", file=sys.stderr)
        
        if magic == b'\x0a\x0d\x0d\x0a':
            print("Format: pcapng", file=sys.stderr)
        elif magic == b'\xd4\xc3\xb2\xa1':
            print("Format: classic pcap (little-endian)", file=sys.stderr)
        elif magic == b'\xa1\xb2\xc3\xd4':
            print("Format: classic pcap (big-endian)", file=sys.stderr)
        else:
            print(f"Format: UNKNOWN", file=sys.stderr)
        
        print("File OK", file=sys.stderr)
except Exception as e:
    print(f"ERROR: {e}", file=sys.stderr)
    sys.exit(1)
