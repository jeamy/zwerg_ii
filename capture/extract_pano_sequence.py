#!/usr/bin/env python3
"""
Extrahiere komplette Panorama-Sequenz aus PCAP
Robuster Parser mit garantierter Ausgabe
"""
import struct
import sys
import json

def log(msg):
    """Garantierte Ausgabe"""
    sys.stdout.write(msg + '\n')
    sys.stdout.flush()

def parse_pcapng(filename):
    """Parse pcapng und extrahiere Pakete"""
    log(f"Reading {filename}...")
    
    try:
        with open(filename, 'rb') as f:
            data = f.read()
    except Exception as e:
        log(f"ERROR reading file: {e}")
        return []
    
    log(f"File size: {len(data)} bytes")
    
    # Check magic
    if len(data) < 4:
        log("ERROR: File too small")
        return []
    
    magic = data[:4]
    log(f"Magic bytes: {magic.hex()}")
    
    if magic != b'\x0a\x0d\x0d\x0a':
        log(f"ERROR: Not a pcapng file (magic: {magic.hex()})")
        return []
    
    log("Format: pcapng ✓")
    
    packets = []
    offset = 0
    block_count = 0
    
    while offset < len(data) - 8:
        try:
            block_type = struct.unpack('<I', data[offset:offset+4])[0]
            block_len = struct.unpack('<I', data[offset+4:offset+8])[0]
            
            if block_len == 0 or block_len > len(data) - offset:
                break
            
            block_count += 1
            
            # Enhanced Packet Block
            if block_type == 0x00000006:
                # Offset structure: type(4) + len(4) + iface(4) + ts_high(4) + ts_low(4) + cap_len(4) + orig_len(4)
                header_offset = offset + 8
                
                if header_offset + 20 <= len(data):
                    captured_len = struct.unpack('<I', data[header_offset+16:header_offset+20])[0]
                    packet_offset = header_offset + 20
                    
                    if packet_offset + captured_len <= len(data):
                        pkt_data = data[packet_offset:packet_offset+captured_len]
                        packets.append(pkt_data)
            
            offset += block_len
        except Exception as e:
            log(f"Error parsing block at offset {offset}: {e}")
            break
    
    log(f"Blocks parsed: {block_count}")
    log(f"Packets extracted: {len(packets)}")
    
    return packets

def extract_tcp_9900(packets):
    """Extrahiere TCP Payloads auf Port 9900"""
    log("\nExtracting TCP payloads on port 9900...")
    
    tcp_payloads = []
    
    for i, pkt in enumerate(packets):
        if len(pkt) < 14:
            continue
        
        # Ethernet header (14 bytes)
        ip_data = pkt[14:]
        
        if len(ip_data) < 20:
            continue
        
        # IP header
        ihl = (ip_data[0] & 0x0F) * 4
        protocol = ip_data[9]
        
        if protocol != 6:  # Not TCP
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
            tcp_payloads.append({
                'pkt_num': i,
                'direction': direction,
                'payload': payload
            })
    
    log(f"TCP payloads found: {len(tcp_payloads)}")
    
    c2s = sum(1 for p in tcp_payloads if p['direction'] == 'C->S')
    s2c = sum(1 for p in tcp_payloads if p['direction'] == 'S->C')
    log(f"  Client->Server: {c2s}")
    log(f"  Server->Client: {s2c}")
    
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

def extract_ws_commands(tcp_payloads):
    """Extrahiere WebSocket Binary Commands"""
    log("\nExtracting WebSocket binary commands...")
    
    ws_commands = []
    
    for tcp_pkt in tcp_payloads:
        ws_result = unmask_websocket(tcp_pkt['payload'])
        
        if ws_result:
            opcode, ws_payload = ws_result
            
            if opcode == 2:  # Binary frame
                ws_commands.append({
                    'pkt_num': tcp_pkt['pkt_num'],
                    'direction': tcp_pkt['direction'],
                    'payload_hex': ws_payload.hex(),
                    'payload_len': len(ws_payload)
                })
    
    log(f"WebSocket binary commands: {len(ws_commands)}")
    
    c2s = [c for c in ws_commands if c['direction'] == 'C->S']
    s2c = [c for c in ws_commands if c['direction'] == 'S->C']
    
    log(f"  Client->Server: {len(c2s)}")
    log(f"  Server->Client: {len(s2c)}")
    
    return c2s, s2c

if __name__ == '__main__':
    if len(sys.argv) < 2:
        log("Usage: python3 extract_pano_sequence.py <pcapng_file>")
        sys.exit(1)
    
    pcap_file = sys.argv[1]
    
    log("="*80)
    log("PANORAMA SEQUENCE EXTRACTOR")
    log("="*80)
    log("")
    
    # Parse
    packets = parse_pcapng(pcap_file)
    
    if not packets:
        log("ERROR: No packets found")
        sys.exit(1)
    
    # Extract TCP
    tcp_payloads = extract_tcp_9900(packets)
    
    if not tcp_payloads:
        log("ERROR: No TCP payloads on port 9900")
        sys.exit(1)
    
    # Extract WebSocket
    c2s_commands, s2c_commands = extract_ws_commands(tcp_payloads)
    
    if not c2s_commands:
        log("ERROR: No WebSocket commands found")
        sys.exit(1)
    
    # Show C2S commands
    log("\n" + "="*80)
    log("CLIENT->SERVER COMMANDS")
    log("="*80)
    
    for i, cmd in enumerate(c2s_commands):
        log(f"\nCommand {i+1}:")
        log(f"  Packet: {cmd['pkt_num']}")
        log(f"  Length: {cmd['payload_len']} bytes")
        log(f"  Hex: {cmd['payload_hex']}")
        
        # Check if it looks like a grid command (64+ bytes)
        if cmd['payload_len'] >= 60:
            payload_bytes = bytes.fromhex(cmd['payload_hex'])
            if len(payload_bytes) > 25:
                log(f"  Offset 15: 0x{payload_bytes[15]:02x} {'(row selector?)' if payload_bytes[15] == 0x9c else '(col selector?)' if payload_bytes[15] == 0x9d else ''}")
                log(f"  Offset 25: 0x{payload_bytes[25]:02x} (value={payload_bytes[25]})")
    
    # Save to JSON
    output = {
        'c2s_commands': c2s_commands,
        's2c_commands': s2c_commands
    }
    
    output_file = pcap_file.replace('.pcapng', '_extracted.json')
    
    try:
        with open(output_file, 'w') as f:
            json.dump(output, f, indent=2)
        log(f"\n✓ Saved to: {output_file}")
    except Exception as e:
        log(f"\nERROR saving JSON: {e}")
    
    log("\nDone.")
