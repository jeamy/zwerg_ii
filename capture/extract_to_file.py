#!/usr/bin/env python3
import struct
import json

output_file = '/media/data/programming/zwergII/capture/extraction_output.txt'
out = open(output_file, 'w')

def log(msg):
    out.write(msg + '\n')
    out.flush()

pcap_file = 'ctrl_20251226_1.pcapng'

log(f"Reading {pcap_file}...")

with open(pcap_file, 'rb') as f:
    data = f.read()

log(f"Size: {len(data)} bytes")
log(f"Magic: {data[:4].hex()}")

if data[:4] != b'\x0a\x0d\x0d\x0a':
    log("ERROR: Not pcapng")
    out.close()
    exit(1)

log("Format: pcapng ✓")

packets = []
offset = 0

while offset < len(data) - 8:
    block_type = struct.unpack('<I', data[offset:offset+4])[0]
    block_len = struct.unpack('<I', data[offset+4:offset+8])[0]
    
    if block_len == 0 or block_len > len(data) - offset:
        break
    
    if block_type == 0x00000006:
        hdr = offset + 8
        if hdr + 20 <= len(data):
            cap_len = struct.unpack('<I', data[hdr+16:hdr+20])[0]
            pkt_off = hdr + 20
            if pkt_off + cap_len <= len(data):
                packets.append(data[pkt_off:pkt_off+cap_len])
    
    offset += block_len

log(f"Packets: {len(packets)}")

# Extract TCP port 9900
tcp_payloads = []

for pkt in packets:
    if len(pkt) < 14:
        continue
    
    ip = pkt[14:]
    if len(ip) < 20:
        continue
    
    ihl = (ip[0] & 0x0F) * 4
    proto = ip[9]
    
    if proto != 6:
        continue
    
    tcp = ip[ihl:]
    if len(tcp) < 20:
        continue
    
    src_port = struct.unpack('>H', tcp[0:2])[0]
    dst_port = struct.unpack('>H', tcp[2:4])[0]
    
    if dst_port != 9900 and src_port != 9900:
        continue
    
    tcp_hdr_len = ((tcp[12] >> 4) & 0x0F) * 4
    payload = tcp[tcp_hdr_len:]
    
    if len(payload) > 0:
        direction = "C->S" if dst_port == 9900 else "S->C"
        tcp_payloads.append({'dir': direction, 'data': payload})

log(f"TCP payloads: {len(tcp_payloads)}")

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

ws_cmds = []

for tcp_pkt in tcp_payloads:
    ws = unmask(tcp_pkt['data'])
    if ws:
        opcode, payload = ws
        if opcode == 2:  # Binary
            ws_cmds.append({
                'dir': tcp_pkt['dir'],
                'hex': payload.hex(),
                'len': len(payload)
            })

log(f"WebSocket binary commands: {len(ws_cmds)}")

c2s = [c for c in ws_cmds if c['dir'] == 'C->S']
log(f"Client->Server: {len(c2s)}")

log("\n" + "="*80)
log("CLIENT->SERVER COMMANDS")
log("="*80)

for i, cmd in enumerate(c2s):
    log(f"\nCommand {i+1}:")
    log(f"  Length: {cmd['len']} bytes")
    log(f"  Hex: {cmd['hex']}")
    
    if cmd['len'] >= 60:
        payload = bytes.fromhex(cmd['hex'])
        if len(payload) > 25:
            log(f"  Offset 15: 0x{payload[15]:02x}")
            log(f"  Offset 25: 0x{payload[25]:02x} (={payload[25]})")

# Save JSON
json_data = {'c2s': c2s, 's2c': [c for c in ws_cmds if c['dir'] == 'S->C']}
with open('ctrl_20251226_1_extracted.json', 'w') as f:
    json.dump(json_data, f, indent=2)

log("\n✓ Saved to ctrl_20251226_1_extracted.json")
log("✓ Done.")

out.close()
