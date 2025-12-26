#!/usr/bin/env python3
"""Parse classic PCAP format (not pcapng)"""
import struct
import json

pcap_file = '/media/data/programming/zwergII/capture/ctrl_20251226_1.pcapng'
output = open('/media/data/programming/zwergII/capture/pano_commands.txt', 'w')

def log(msg):
    output.write(msg + '\n')
    output.flush()

with open(pcap_file, 'rb') as f:
    data = f.read()

log(f"File: {pcap_file}")
log(f"Size: {len(data)} bytes")
log(f"Magic: {data[:4].hex()}")

magic = struct.unpack('<I', data[0:4])[0]

if magic == 0xa1b2c3d4:
    endian = '<'
    log("Format: Classic PCAP (little endian)")
elif magic == 0xd4c3b2a1:
    endian = '>'
    log("Format: Classic PCAP (big endian)")
else:
    log(f"ERROR: Unknown format (magic={magic:08x})")
    output.close()
    exit(1)

# Skip global header (24 bytes)
offset = 24
packets = []

while offset < len(data) - 16:
    # Packet header: ts_sec(4), ts_usec(4), incl_len(4), orig_len(4)
    incl_len = struct.unpack(endian+'I', data[offset+8:offset+12])[0]
    
    pkt_data = data[offset+16:offset+16+incl_len]
    packets.append(pkt_data)
    
    offset += 16 + incl_len

log(f"Packets: {len(packets)}")

# Extract TCP port 9900
tcp_payloads = []

for pkt in packets:
    if len(pkt) < 14:
        continue
    
    ip_data = pkt[14:]
    if len(ip_data) < 20:
        continue
    
    ihl = (ip_data[0] & 0x0F) * 4
    protocol = ip_data[9]
    
    if protocol != 6:
        continue
    
    tcp_data = ip_data[ihl:]
    if len(tcp_data) < 20:
        continue
    
    src_port = struct.unpack('>H', tcp_data[0:2])[0]
    dst_port = struct.unpack('>H', tcp_data[2:4])[0]
    
    if dst_port != 9900 and src_port != 9900:
        continue
    
    tcp_hdr_len = ((tcp_data[12] >> 4) & 0x0F) * 4
    payload = tcp_data[tcp_hdr_len:]
    
    if len(payload) > 0:
        direction = "C->S" if dst_port == 9900 else "S->C"
        tcp_payloads.append({'dir': direction, 'data': payload})

log(f"TCP port 9900 payloads: {len(tcp_payloads)}")

# Unmask WebSocket
def unmask(data):
    if len(data) < 2:
        return None
    opcode = data[0] & 0x0F
    masked = (data[1] & 0x80) != 0
    plen = data[1] & 0x7F
    off = 2
    if plen == 126:
        if len(data) < off + 2:
            return None
        plen = struct.unpack('>H', data[off:off+2])[0]
        off += 2
    elif plen == 127:
        if len(data) < off + 8:
            return None
        plen = struct.unpack('>Q', data[off:off+8])[0]
        off += 8
    if masked:
        if len(data) < off + 4:
            return None
        mask = data[off:off+4]
        off += 4
        if len(data) < off + plen:
            return None
        payload = bytearray(data[off:off+plen])
        for i in range(len(payload)):
            payload[i] ^= mask[i % 4]
        return (opcode, bytes(payload))
    else:
        if len(data) < off + plen:
            return None
        return (opcode, data[off:off+plen])

ws_commands = []
for tcp_pkt in tcp_payloads:
    ws = unmask(tcp_pkt['data'])
    if ws and ws[0] == 2:
        ws_commands.append({'dir': tcp_pkt['dir'], 'payload': ws[1]})

log(f"WebSocket binary commands: {len(ws_commands)}")

c2s = [c for c in ws_commands if c['dir'] == 'C->S']
s2c = [c for c in ws_commands if c['dir'] == 'S->C']

log(f"  C->S: {len(c2s)}")
log(f"  S->C: {len(s2c)}")

log("\n" + "="*80)
log("CLIENT->SERVER COMMANDS:")
log("="*80)

for i, cmd in enumerate(c2s):
    payload = cmd['payload']
    log(f"\nCommand {i+1}:")
    log(f"  Length: {len(payload)} bytes")
    log(f"  Hex: {payload.hex()}")
    
    if len(payload) >= 26:
        selector_note = ""
        if payload[15] == 0x9c:
            selector_note = " <- ROW selector"
        elif payload[15] == 0x9d:
            selector_note = " <- COL selector"
        
        log(f"  Offset[15]: 0x{payload[15]:02x}{selector_note}")
        log(f"  Offset[25]: 0x{payload[25]:02x} (decimal={payload[25]})")

# Save to JSON
json_data = {
    'c2s_commands': [{'payload_hex': c['payload'].hex(), 'payload_len': len(c['payload'])} for c in c2s],
    's2c_commands': [{'payload_hex': c['payload'].hex(), 'payload_len': len(c['payload'])} for c in s2c]
}

with open('/media/data/programming/zwergII/capture/ctrl_20251226_1_commands.json', 'w') as f:
    json.dump(json_data, f, indent=2)

log("\n✓ Saved JSON to: ctrl_20251226_1_commands.json")
log("✓ Done.")

output.close()
