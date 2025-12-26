#!/usr/bin/env python3
"""
Extract WebSocket Handshake (HTTP Upgrade) from PCAP
Analyze HTTP headers sent by Android app
"""
import struct

pcap_file = '/media/data/programming/zwergII/capture/ctrl_20251226_113958.pcapng'
output = open('/media/data/programming/zwergII/capture/ws_handshake.txt', 'w')

def log(msg):
    output.write(msg + '\n')
    output.flush()

with open(pcap_file, 'rb') as f:
    data = f.read()

log("="*80)
log("WEBSOCKET HANDSHAKE EXTRACTION")
log("="*80)
log("")

# Parse PCAP
magic = struct.unpack('<I', data[0:4])[0]
if magic == 0xd4c3b2a1:
    endian = '>'
else:
    endian = '<'

offset = 24
packets = []

while offset < len(data) - 16:
    incl_len = struct.unpack(endian+'I', data[offset+8:offset+12])[0]
    pkt_data = data[offset+16:offset+16+incl_len]
    packets.append(pkt_data)
    offset += 16 + incl_len

log(f"Total packets: {len(packets)}")

# Look for HTTP traffic (port 9900)
http_packets = []

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
        # Check if payload looks like HTTP
        if payload[:4] == b'GET ' or payload[:4] == b'HTTP':
            http_packets.append({
                'idx': idx,
                'src_port': src_port,
                'dst_port': dst_port,
                'payload': payload
            })

log(f"HTTP packets found: {len(http_packets)}")
log("")

if not http_packets:
    log("ERROR: No HTTP packets found!")
    log("WebSocket might already be established or using different handshake method.")
    output.close()
    exit(0)

# Extract HTTP headers
log("="*80)
log("HTTP TRAFFIC")
log("="*80)

for i, pkt in enumerate(http_packets):
    log(f"\nPacket {pkt['idx']} | {pkt['src_port']} -> {pkt['dst_port']}")
    log("-" * 80)
    
    payload = pkt['payload']
    
    # Try to decode as ASCII/UTF-8
    try:
        text = payload.decode('utf-8', errors='replace')
        log(text)
    except:
        log(f"Binary data: {payload[:100].hex()}")
    
    log("")

# Look specifically for WebSocket Upgrade
log("="*80)
log("WEBSOCKET UPGRADE ANALYSIS")
log("="*80)
log("")

for pkt in http_packets:
    payload = pkt['payload']
    
    try:
        text = payload.decode('utf-8', errors='replace')
        
        if 'Upgrade' in text and 'websocket' in text.lower():
            log("✓ WebSocket Upgrade Request found!")
            log("")
            log("Full Request:")
            log("-" * 80)
            log(text)
            log("-" * 80)
            log("")
            
            # Extract key headers
            lines = text.split('\r\n')
            
            log("Key Headers:")
            for line in lines:
                if any(keyword in line for keyword in ['Upgrade:', 'Connection:', 'Sec-WebSocket', 'Host:', 'Origin:', 'User-Agent:', 'Cookie:', 'Authorization:', 'X-']):
                    log(f"  {line}")
            
            break
    except:
        pass

log("")
log("✓ Done.")
output.close()
