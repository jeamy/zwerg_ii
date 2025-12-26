#!/usr/bin/env python3
"""
Decode module_id and cmd from PCAP commands
To understand exact values for C++ implementation
"""
import sys
sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

# Commands from ctrl_20251226_113958.pcapng
commands = {
    'Panorama UI Open': "080110141801200e289280013a020807422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    'ROW=5': "080110141801200f28bf82013a0c089c8080808080bc81071005422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    'COL=5': "080110141801200f28bf82013a0c089d8080808080bc81071005422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    'START': "080110141801200a288c79422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
}

def varint_decode(data, offset=0):
    result = 0
    shift = 0
    while offset < len(data):
        byte = data[offset]
        result |= (byte & 0x7F) << shift
        offset += 1
        if (byte & 0x80) == 0:
            break
        shift += 7
    return result, offset

print("="*80)
print("COMMAND DECODE - Module/CMD extraction")
print("="*80)
print()

for name, hex_str in commands.items():
    payload = bytes.fromhex(hex_str)
    
    # Parse as WsPacket
    pkt = proto.WsPacket()
    pkt.ParseFromString(payload)
    
    print(f"{name}:")
    print(f"  Module: {pkt.module_id}")
    print(f"  CMD:    {pkt.cmd}")
    print(f"  Type:   {pkt.type}")
    print(f"  Data length: {len(pkt.data)} bytes")
    if len(pkt.data) <= 50:
        print(f"  Data hex: {pkt.data.hex()}")
    print()
