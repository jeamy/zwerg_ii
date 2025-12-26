#!/usr/bin/env python3
"""
Extrahiere ALLE WebSocket Binary Commands aus PCAP
Unmask Client->Server Payloads und speichere sie zum Replay
"""
import struct
import json

def read_pcapng(filename):
    """Read pcapng file and extract TCP payloads"""
    packets = []
    
    with open(filename, 'rb') as f:
        data = f.read()
    
    offset = 0
    while offset < len(data):
        if offset + 8 > len(data):
            break
        
        block_type = struct.unpack('<I', data[offset:offset+4])[0]
        block_len = struct.unpack('<I', data[offset+4:offset+8])[0]
        
        if block_len == 0 or block_len > len(data) - offset:
            break
        
        # Enhanced Packet Block
        if block_type == 0x00000006:
            packet_offset = offset + 8 + 4 + 8 + 4 + 4
            captured_len = struct.unpack('<I', data[offset+8+4+8:offset+8+4+8+4])[0]
            
            if packet_offset + captured_len <= len(data):
                packet_data = data[packet_offset:packet_offset+captured_len]
                packets.append(packet_data)
        
        offset += block_len
    
    return packets

def parse_tcp_stream(packets):
    """Extract TCP payloads on port 9900"""
    tcp_payloads = []
    
    for pkt in packets:
        if len(pkt) < 14:
            continue
        
        # Skip Ethernet (14 bytes)
        ip_data = pkt[14:]
        
        if len(ip_data) < 20:
            continue
        
        # IP header length
        ihl = (ip_data[0] & 0x0F) * 4
        protocol = ip_data[9]
        
        if protocol != 6:  # Not TCP
            continue
        
        src_ip = '.'.join(str(b) for b in ip_data[12:16])
        dst_ip = '.'.join(str(b) for b in ip_data[16:20])
        
        tcp_data = ip_data[ihl:]
        
        if len(tcp_data) < 20:
            continue
        
        src_port = struct.unpack('>H', tcp_data[0:2])[0]
        dst_port = struct.unpack('>H', tcp_data[2:4])[0]
        
        if dst_port != 9900 and src_port != 9900:
            continue
        
        tcp_hdr_len = ((tcp_data[12] >> 4) & 0x0F) * 4
        
        if len(tcp_data) < tcp_hdr_len:
            continue
        
        payload = tcp_data[tcp_hdr_len:]
        
        if len(payload) > 0:
            direction = "C2S" if dst_port == 9900 else "S2C"
            tcp_payloads.append({
                'direction': direction,
                'src': f"{src_ip}:{src_port}",
                'dst': f"{dst_ip}:{dst_port}",
                'payload': payload
            })
    
    return tcp_payloads

def unmask_websocket(data):
    """Unmask WebSocket frame"""
    if len(data) < 2:
        return None
    
    opcode = data[0] & 0x0F
    masked = (data[1] & 0x80) != 0
    payload_len = data[1] & 0x7F
    
    offset = 2
    
    if payload_len == 126:
        if len(data) < offset + 2:
            return None
        payload_len = struct.unpack('>H', data[offset:offset+2])[0]
        offset += 2
    elif payload_len == 127:
        if len(data) < offset + 8:
            return None
        payload_len = struct.unpack('>Q', data[offset:offset+8])[0]
        offset += 8
    
    if masked:
        if len(data) < offset + 4:
            return None
        mask = data[offset:offset+4]
        offset += 4
        
        if len(data) < offset + payload_len:
            return None
        
        payload = bytearray(data[offset:offset+payload_len])
        for i in range(len(payload)):
            payload[i] ^= mask[i % 4]
        
        return (opcode, bytes(payload))
    else:
        if len(data) < offset + payload_len:
            return None
        return (opcode, data[offset:offset+payload_len])

def extract_commands(pcap_file):
    """Extract all WebSocket binary commands from PCAP"""
    print(f"Reading {pcap_file}...")
    packets = read_pcapng(pcap_file)
    print(f"Total packets: {len(packets)}")
    
    tcp_payloads = parse_tcp_stream(packets)
    print(f"TCP payloads on port 9900: {len(tcp_payloads)}")
    
    commands = []
    
    for i, tcp_pkt in enumerate(tcp_payloads):
        direction = tcp_pkt['direction']
        payload = tcp_pkt['payload']
        
        # Try to parse as WebSocket
        ws_result = unmask_websocket(payload)
        if not ws_result:
            continue
        
        opcode, ws_payload = ws_result
        
        # Binary frame (opcode 2)
        if opcode == 2:
            commands.append({
                'index': i,
                'direction': direction,
                'opcode': opcode,
                'payload_hex': ws_payload.hex(),
                'payload_len': len(ws_payload)
            })
    
    return commands

if __name__ == '__main__':
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python3 extract_all_ws_commands.py <pcapng_file>")
        sys.exit(1)
    
    pcap_file = sys.argv[1]
    commands = extract_commands(pcap_file)
    
    print(f"\nExtracted {len(commands)} WebSocket binary commands")
    print()
    
    # Save to JSON
    output_file = pcap_file.replace('.pcapng', '_commands.json')
    with open(output_file, 'w') as f:
        json.dump(commands, f, indent=2)
    
    print(f"Saved to: {output_file}")
    print()
    
    # Print summary
    c2s_count = sum(1 for c in commands if c['direction'] == 'C2S')
    s2c_count = sum(1 for c in commands if c['direction'] == 'S2C')
    
    print(f"Client->Server: {c2s_count}")
    print(f"Server->Client: {s2c_count}")
    print()
    
    # Show first few C2S commands
    print("First 10 Client->Server commands:")
    for i, cmd in enumerate([c for c in commands if c['direction'] == 'C2S'][:10]):
        print(f"{i+1}. Index {cmd['index']}, {cmd['payload_len']} bytes, hex: {cmd['payload_hex'][:60]}...")
