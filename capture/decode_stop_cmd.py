#!/usr/bin/env python3
"""
Decode START and STOP commands from Android PCAP
"""
import sys
sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

# From ctrl_20251226_1.pcapng
START = "080110141801200a288c79422432303939643762392d323537612d343166632d613161622d376535316165326630333030"
STOP  = "080110141801200a288d79422432303939643762392d323537612d343166632d613161622d376535316165326630333030"

print("="*80)
print("START vs STOP Command Analysis")
print("="*80)
print()

for name, hex_str in [("START", START), ("STOP", STOP)]:
    data = bytes.fromhex(hex_str)
    pkt = proto.WsPacket()
    pkt.ParseFromString(data)
    
    print(f"{name} Command:")
    print(f"  Total length: {len(data)} bytes")
    print(f"  Module: {pkt.module_id}")
    print(f"  CMD: {pkt.cmd}")
    print(f"  Type: {pkt.type}")
    print(f"  Data field length: {len(pkt.data)} bytes")
    if len(pkt.data) > 0:
        print(f"  Data hex: {pkt.data.hex()}")
    else:
        print(f"  Data: EMPTY")
    print()

print("="*80)
print("CONCLUSION")
print("="*80)

start_data = bytes.fromhex(START)
stop_data = bytes.fromhex(STOP)

if len(start_data) == len(stop_data):
    print(f"✓ Same length: {len(start_data)} bytes")
else:
    print(f"✗ Different lengths: START={len(start_data)}, STOP={len(stop_data)}")

# Find differences
diffs = []
for i in range(min(len(start_data), len(stop_data))):
    if start_data[i] != stop_data[i]:
        diffs.append((i, start_data[i], stop_data[i]))

print()
if diffs:
    print(f"Differences at {len(diffs)} byte(s):")
    for offset, start_b, stop_b in diffs:
        print(f"  Offset {offset}: START=0x{start_b:02x} STOP=0x{stop_b:02x}")
else:
    print("✗ Payloads are identical!")

print()
print("Both commands have EMPTY data field → No Protobuf payload needed!")
