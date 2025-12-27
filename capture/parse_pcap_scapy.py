#!/usr/bin/env python3
"""
Parse PCAP mit Scapy (falls verfügbar) oder rohem Parser
Extrahiere ALLE WebSocket Commands
"""
import struct
import sys

def parse_pcap_raw(filename):
    """Parse PCAP ohne externe Tools"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Check PCAP format
    if data[:4] == b'\x0a\x0d\x0d\x0a':
        return parse_pcapng_raw(data)
    elif data[:4] in (b'\xd4\xc3\xb2\xa1', b'\xa1\xb2\xc3\xd4'):
        return parse_pcap_classic(data)
    else:
        print(f"Unknown format: {data[:4].hex()}")
        return []

def parse_pcapng_raw(data):
    """Parse pcapng format"""
    packets = []
    offset = 0
    
    while offset < len(data) - 8:
        block_type = struct.unpack('<I', data[offset:offset+4])[0]
        block_len = struct.unpack('<I', data[offset+4:offset+8])[0]
        
        if block_len == 0 or block_len > len(data) - offset:
            break
        
        # Enhanced Packet Block (0x06)
        if block_type == 0x00000006:
            try:
                # offset+8: interface_id(4), timestamp_high(4), timestamp_low(4), 
                # captured_len(4), original_len(4), packet_data
                pkt_offset = offset + 8 + 4 + 8
                captured_len = struct.unpack('<I', data[pkt_offset:pkt_offset+4])[0]
                pkt_offset += 8  # skip captured_len and original_len
                
                if pkt_offset + captured_len <= len(data):
                    pkt_data = data[pkt_offset:pkt_offset+captured_len]
                    packets.append(pkt_data)
            except:
                pass
        
        offset += block_len
    
    return packets

def parse_pcap_classic(data):
    """Parse classic pcap format"""
    packets = []
    
    # Read global header
    magic = struct.unpack('<I', data[0:4])[0]
    if magic == 0xa1b2c3d4:
        endian = '<'
    else:
        endian = '>'
    
    offset = 24  # Skip global header
    
    while offset < len(data) - 16:
        # Packet header: ts_sec(4), ts_usec(4), incl_len(4), orig_len(4)
        try:
            incl_len = struct.unpack(endian+'I', data[offset+8:offset+12])[0]
            pkt_data = data[offset+16:offset+16+incl_len]
            packets.append(pkt_data)
            offset += 16 + incl_len
        except:
            break
    
    return packets

def extract_tcp_payload(pkt_data):
    """Extract TCP payload from Ethernet frame"""
    if len(pkt_data) < 14:
        return None
    
    # Skip Ethernet (14 bytes)
    ip_data = pkt_data[14:]
    
    if len(ip_data) < 20:
        return None
    
    # IP header
    ihl = (ip_data[0] & 0x0F) * 4
    protocol = ip_data[9]
    
    if protocol != 6:  # Not TCP
        return None
    
    src_ip = '.'.join(str(b) for b in ip_data[12:16])
    dst_ip = '.'.join(str(b) for b in ip_data[16:20])
    
    tcp_data = ip_data[ihl:]
    
    if len(tcp_data) < 20:
        return None
    
    src_port = struct.unpack('>H', tcp_data[0:2])[0]
    dst_port = struct.unpack('>H', tcp_data[2:4])[0]
    
    if dst_port != 9900 and src_port != 9900:
        return None
    
    tcp_hdr_len = ((tcp_data[12] >> 4) & 0x0F) * 4
    payload = tcp_data[tcp_hdr_len:]
    
    if len(payload) == 0:
        return None
    
    direction = "C2S" if dst_port == 9900 else "S2C"
    
    return {
        'direction': direction,
        'src': f"{src_ip}:{src_port}",
        'dst': f"{dst_ip}:{dst_port}",
        'payload': payload
    }

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

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 parse_pcap_scapy.py <pcap_file>")
        sys.exit(1)
    
    pcap_file = sys.argv[1]
    
    print(f"Parsing {pcap_file}...")
    packets = parse_pcap_raw(pcap_file)
    print(f"Total packets: {len(packets)}")
    
    tcp_payloads = []
    for pkt in packets:
        result = extract_tcp_payload(pkt)
        if result:
            tcp_payloads.append(result)
    
    print(f"TCP payloads on port 9900: {len(tcp_payloads)}")
    
    ws_commands = []
    for tcp_pkt in tcp_payloads:
        ws_result = unmask_websocket(tcp_pkt['payload'])
        if ws_result:
            opcode, ws_payload = ws_result
            if opcode == 2:  # Binary
                ws_commands.append({
                    'direction': tcp_pkt['direction'],
                    'opcode': opcode,
                    'payload': ws_payload
                })
    
    print(f"WebSocket binary commands: {len(ws_commands)}")
    print()
    
    c2s_cmds = [c for c in ws_commands if c['direction'] == 'C2S']
    print(f"Client->Server commands: {len(c2s_cmds)}")
    print()
    
    for i, cmd in enumerate(c2s_cmds):
        payload = cmd['payload']
        print(f"Command {i+1}:")
        print(f"  Length: {len(payload)} bytes")
        print(f"  Hex: {payload.hex()}")
        print(f"  Head: {payload[:12].hex()}")
        print()
