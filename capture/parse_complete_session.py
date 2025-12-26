#!/usr/bin/env python3
"""Parse COMPLETE Android session with app init"""
import struct
import json

pcap_file = '/media/data/programming/zwergII/capture/ctrl_20251226_113958.pcapng'
output = open('/media/data/programming/zwergII/capture/complete_session.txt', 'w')

def log(msg):
    output.write(msg + '\n')
    output.flush()

with open(pcap_file, 'rb') as f:
    data = f.read()

log(f"File: {pcap_file}")
log(f"Size: {len(data)} bytes")
log(f"Magic: {data[:4].hex()}")

# Detect format
magic = struct.unpack('<I', data[0:4])[0]

if magic == 0xa1b2c3d4:
    endian = '<'
    log("Format: Classic PCAP (little endian)")
elif magic == 0xd4c3b2a1:
    endian = '>'
    log("Format: Classic PCAP (big endian)")
elif data[:4] == b'\x0a\x0d\x0d\x0a':
    log("Format: pcapng")
    # Parse pcapng
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
else:
    log(f"ERROR: Unknown format")
    output.close()
    exit(1)

# Classic PCAP
if magic in (0xa1b2c3d4, 0xd4c3b2a1):
    offset = 24
    packets = []
    while offset < len(data) - 16:
        incl_len = struct.unpack(endian+'I', data[offset+8:offset+12])[0]
        pkt_data = data[offset+16:offset+16+incl_len]
        packets.append(pkt_data)
        offset += 16 + incl_len

log(f"Packets: {len(packets)}")

# Extract TCP port 9900
tcp_payloads = []

for idx, pkt in enumerate(packets):
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
        tcp_payloads.append({'idx': idx, 'dir': direction, 'data': payload})

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
    if ws and ws[0] == 2:  # Binary
        ws_commands.append({
            'pkt_idx': tcp_pkt['idx'],
            'dir': tcp_pkt['dir'],
            'payload': ws[1]
        })

log(f"WebSocket binary commands: {len(ws_commands)}")

c2s = [c for c in ws_commands if c['dir'] == 'C->S']
s2c = [c for c in ws_commands if c['dir'] == 'S->C']

log(f"  C->S: {len(c2s)}")
log(f"  S->C: {len(s2c)}")

log("\n" + "="*80)
log("CLIENT->SERVER COMMANDS (ALL)")
log("="*80)

for i, cmd in enumerate(c2s):
    payload = cmd['payload']
    log(f"\nCommand {i+1} (packet {cmd['pkt_idx']}):")
    log(f"  Length: {len(payload)} bytes")
    log(f"  Hex: {payload.hex()}")
    
    # Detect command type
    if len(payload) >= 26:
        if payload[15] == 0x9c:
            log(f"  → ROW selector (offset[15]=0x9c), value={payload[25]}")
        elif payload[15] == 0x9d:
            log(f"  → COL selector (offset[15]=0x9d), value={payload[25]}")
    
    # Detect module/cmd if possible (basic heuristic)
    if len(payload) >= 10:
        # WsPacket: varint_len + module_id(field 1) + cmd(field 2)
        # Try to decode first few bytes
        try:
            off = 0
            # Skip varint length prefix
            while off < len(payload) and (payload[off] & 0x80):
                off += 1
            off += 1
            
            if off < len(payload):
                # Try to decode fields
                fields_desc = []
                for _ in range(5):  # Try first 5 fields
                    if off >= len(payload):
                        break
                    tag = payload[off]
                    field_num = tag >> 3
                    wire_type = tag & 0x7
                    off += 1
                    
                    if wire_type == 0:  # varint
                        val = 0
                        shift = 0
                        while off < len(payload):
                            b = payload[off]
                            val |= (b & 0x7F) << shift
                            off += 1
                            if (b & 0x80) == 0:
                                break
                            shift += 7
                        fields_desc.append(f"f{field_num}={val}")
                    else:
                        break
                
                if fields_desc:
                    log(f"  Fields: {', '.join(fields_desc)}")
        except:
            pass

# Save JSON
json_data = {
    'c2s_commands': [{'pkt_idx': c['pkt_idx'], 'payload_hex': c['payload'].hex(), 'payload_len': len(c['payload'])} for c in c2s],
    's2c_commands': [{'pkt_idx': c['pkt_idx'], 'payload_hex': c['payload'].hex(), 'payload_len': len(c['payload'])} for c in s2c]
}

with open('/media/data/programming/zwergII/capture/ctrl_20251226_113958_full.json', 'w') as f:
    json.dump(json_data, f, indent=2)

log("\n✓ Saved JSON to: ctrl_20251226_113958_full.json")
log("✓ Done.")

output.close()
