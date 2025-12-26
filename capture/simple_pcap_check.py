#!/usr/bin/env python3
import struct
import sys

pcap = sys.argv[1] if len(sys.argv) > 1 else "ctrl_20251225_114040.pcapng"

with open(pcap, 'rb') as f:
    data = f.read()

print(f"File: {pcap}")
print(f"Size: {len(data)} bytes")
print(f"Magic: {data[:4].hex()}")

if data[:4] == b'\x0a\x0d\x0d\x0a':
    print("Format: pcapng ✓")
elif data[:4] in (b'\xd4\xc3\xb2\xa1', b'\xa1\xb2\xc3\xd4'):
    print("Format: pcap classic ✓")
else:
    print("Format: UNKNOWN")
    sys.exit(1)

# Count blocks
block_count = 0
offset = 0
while offset < len(data) - 8:
    try:
        block_type = struct.unpack('<I', data[offset:offset+4])[0]
        block_len = struct.unpack('<I', data[offset+4:offset+8])[0]
        
        if block_len == 0 or block_len > len(data) - offset:
            break
        
        block_count += 1
        offset += block_len
    except:
        break

print(f"Blocks: {block_count}")
