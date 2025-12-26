#!/usr/bin/env python3
"""
Direkte PCAP-Analyse ohne tshark - extrahiert WebSocket payloads
und sucht nach Panorama-Row/Col Commands
"""
import struct
import sys

def read_pcapng(filename):
    """Read pcapng file and extract TCP payloads on port 9900"""
    packets = []
    
    with open(filename, 'rb') as f:
        data = f.read()
    
    offset = 0
    while offset < len(data):
        if offset + 8 > len(data):
            break
        
        # Read block type and length
        block_type = struct.unpack('<I', data[offset:offset+4])[0]
        block_len = struct.unpack('<I', data[offset+4:offset+8])[0]
        
        if block_len == 0 or block_len > len(data) - offset:
            break
        
        # Enhanced Packet Block (EPB) = 0x00000006
        if block_type == 0x00000006:
            # Skip interface ID (4), timestamp (8), captured len (4), original len (4)
            packet_offset = offset + 8 + 4 + 8 + 4 + 4
            captured_len = struct.unpack('<I', data[offset+8+4+8:offset+8+4+8+4])[0]
            
            if packet_offset + captured_len <= len(data):
                packet_data = data[packet_offset:packet_offset+captured_len]
                packets.append(packet_data)
        
        offset += block_len
    
    return packets

def parse_ethernet(data):
    """Parse Ethernet frame"""
    if len(data) < 14:
        return None
    
    # Skip to IP header (skip Ethernet header: 14 bytes)
    return data[14:]

def parse_ip(data):
    """Parse IP packet, return (src_ip, dst_ip, protocol, payload)"""
    if len(data) < 20:
        return None
    
    # IP header length (4 bits in first byte)
    ihl = (data[0] & 0x0F) * 4
    protocol = data[9]
    
    src_ip = '.'.join(str(b) for b in data[12:16])
    dst_ip = '.'.join(str(b) for b in data[16:20])
    
    if len(data) < ihl:
        return None
    
    return (src_ip, dst_ip, protocol, data[ihl:])

def parse_tcp(data):
    """Parse TCP segment, return (src_port, dst_port, payload)"""
    if len(data) < 20:
        return None
    
    src_port = struct.unpack('>H', data[0:2])[0]
    dst_port = struct.unpack('>H', data[2:4])[0]
    
    # TCP header length (4 bits in offset 12, upper nibble)
    tcp_hdr_len = ((data[12] >> 4) & 0x0F) * 4
    
    if len(data) < tcp_hdr_len:
        return None
    
    payload = data[tcp_hdr_len:]
    return (src_port, dst_port, payload)

def unmask_websocket(data):
    """Unmask WebSocket frame if masked"""
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

def varint_decode(data, offset=0):
    """Decode protobuf varint"""
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

def parse_ws_packet(payload):
    """Parse DWARF WsPacket protobuf"""
    if len(payload) < 2:
        return None
    
    # First is varint length prefix
    pkt_len, offset = varint_decode(payload, 0)
    
    if offset + pkt_len > len(payload):
        return None
    
    packet_data = payload[offset:offset+pkt_len]
    
    # Parse WsPacket fields
    module_id = None
    cmd = None
    pkt_type = None
    data_payload = None
    
    off = 0
    while off < len(packet_data):
        tag, off = varint_decode(packet_data, off)
        field_num = tag >> 3
        wire_type = tag & 0x7
        
        if wire_type == 0:  # Varint
            value, off = varint_decode(packet_data, off)
            if field_num == 1:
                module_id = value
            elif field_num == 2:
                cmd = value
            elif field_num == 3:
                pkt_type = value
        elif wire_type == 2:  # Length-delimited
            length, off = varint_decode(packet_data, off)
            if off + length > len(packet_data):
                break
            if field_num == 4:
                data_payload = packet_data[off:off+length]
            off += length
        else:
            break
    
    return {
        'module_id': module_id,
        'cmd': cmd,
        'type': pkt_type,
        'data': data_payload,
        'raw': payload
    }

def analyze_pcap(filename):
    """Analyze PCAP for panorama commands"""
    print(f"\n{'='*80}")
    print(f"Analyzing: {filename}")
    print(f"{'='*80}\n")
    
    packets = read_pcapng(filename)
    print(f"Total blocks: {len(packets)}")
    
    ws_packets = []
    
    for i, pkt in enumerate(packets):
        # Parse Ethernet -> IP -> TCP
        ip_data = parse_ethernet(pkt)
        if not ip_data:
            continue
        
        ip_info = parse_ip(ip_data)
        if not ip_info or ip_info[2] != 6:  # Not TCP
            continue
        
        src_ip, dst_ip, protocol, tcp_data = ip_info
        
        tcp_info = parse_tcp(tcp_data)
        if not tcp_info:
            continue
        
        src_port, dst_port, tcp_payload = tcp_info
        
        # Filter for WebSocket traffic on port 9900
        if dst_port != 9900 and src_port != 9900:
            continue
        
        if len(tcp_payload) == 0:
            continue
        
        # Try to parse as WebSocket
        ws_result = unmask_websocket(tcp_payload)
        if not ws_result:
            continue
        
        opcode, ws_payload = ws_result
        
        # Binary frame (opcode 2)
        if opcode == 2:
            # Try to parse as DWARF WsPacket
            pkt_info = parse_ws_packet(ws_payload)
            if pkt_info and pkt_info['module_id'] is not None:
                direction = "C->S" if dst_port == 9900 else "S->C"
                ws_packets.append({
                    'direction': direction,
                    'src': f"{src_ip}:{src_port}",
                    'dst': f"{dst_ip}:{dst_port}",
                    **pkt_info
                })
    
    print(f"WebSocket packets found: {len(ws_packets)}\n")
    
    # Look for panorama-related commands
    pano_packets = []
    for pkt in ws_packets:
        mod = pkt['module_id']
        cmd = pkt['cmd']
        
        # Module 10 = Panorama, Module 14 = Panorama UI, Module 15 = ?
        # Module 1 = Camera Tele (Feature Params)
        if mod in (1, 10, 14, 15) or (mod == 10 and cmd in (15500, 15501, 15219)):
            pano_packets.append(pkt)
    
    print(f"Panorama-related packets: {len(pano_packets)}\n")
    
    for pkt in pano_packets:
        direction = pkt['direction']
        mod = pkt['module_id']
        cmd = pkt['cmd']
        pkt_type = pkt['type']
        data = pkt['data']
        
        print(f"[{direction}] Module {mod:2d}, CMD {cmd:5d}, Type {pkt_type}")
        
        if data:
            print(f"  Data ({len(data)} bytes): {data.hex()}")
            
            # Try to decode special cases
            if mod == 15 and cmd == 16703:
                print(f"  → PANORAMA GRID PARAM (Module 15, CMD 16703)")
                # Parse field 1 (selector) and field 2 (value)
                off = 0
                selector = None
                value = None
                while off < len(data):
                    tag, off = varint_decode(data, off)
                    field_num = tag >> 3
                    wire_type = tag & 0x7
                    if wire_type == 0:
                        val, off = varint_decode(data, off)
                        if field_num == 1:
                            selector = val
                        elif field_num == 2:
                            value = val
                
                if selector is not None and value is not None:
                    print(f"     Selector: {selector} (0x{selector:x})")
                    print(f"     Value: {value}")
                    
                    # Check if it matches known selectors
                    if selector == 0xe05e00000001c:
                        print(f"     → ROWS = {value}")
                    elif selector == 0xe05e00000001d:
                        print(f"     → COLS = {value}")
            
            elif mod == 1 and cmd == 10037:
                print(f"  → FEATURE PARAM (Module 1, CMD 10037)")
                # Parse CommonParam
                off = 0
                while off < len(data):
                    tag, off = varint_decode(data, off)
                    field_num = tag >> 3
                    wire_type = tag & 0x7
                    
                    if wire_type == 2:  # Length-delimited (nested message)
                        length, off = varint_decode(data, off)
                        if field_num == 1:  # param field
                            nested = data[off:off+length]
                            print(f"     CommonParam: {nested.hex()}")
                            # Parse nested fields
                            off2 = 0
                            feature_id = None
                            mode_idx = None
                            cont_val = None
                            while off2 < len(nested):
                                tag2, off2 = varint_decode(nested, off2)
                                fn2 = tag2 >> 3
                                wt2 = tag2 & 0x7
                                if wt2 == 0:
                                    v, off2 = varint_decode(nested, off2)
                                    if fn2 == 3:
                                        feature_id = v
                                    elif fn2 == 4:
                                        mode_idx = v
                                elif wt2 == 1:  # 64-bit (double)
                                    if fn2 == 6:
                                        val_bytes = nested[off2:off2+8]
                                        cont_val = struct.unpack('<d', val_bytes)[0]
                                        off2 += 8
                            
                            if feature_id is not None:
                                print(f"     Feature ID: {feature_id}")
                                if feature_id == 6:
                                    print(f"     → PANO ROWS = {cont_val}")
                                elif feature_id == 7:
                                    print(f"     → PANO COLS = {cont_val}")
                        off += length
            
            elif mod == 10 and cmd == 15500:
                print(f"  → PANORAMA START")
            elif mod == 14 and cmd == 16402:
                print(f"  → PANORAMA UI OPEN")
        
        print()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_pano_pcap.py <pcapng_file> [<pcapng_file2> ...]")
        sys.exit(1)
    
    for pcap_file in sys.argv[1:]:
        try:
            analyze_pcap(pcap_file)
        except Exception as e:
            print(f"Error analyzing {pcap_file}: {e}")
            import traceback
            traceback.print_exc()
