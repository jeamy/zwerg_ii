#!/usr/bin/env python3
"""
Parse zwergII PCAP to extract WebSocket commands
Specifically looking for Module 20, CMD 16703 (grid params)
"""
import sys
import struct

def read_pcapng(filename):
    """Read pcapng file and extract packets"""
    packets = []
    with open(filename, 'rb') as f:
        # Read magic bytes
        magic = f.read(4)
        if magic == b'\x0a\x0d\x0d\x0a':
            # pcapng format
            f.seek(0)
            while True:
                block_type = f.read(4)
                if len(block_type) < 4:
                    break
                
                block_len = struct.unpack('<I', f.read(4))[0]
                block_data = f.read(block_len - 12)
                f.read(4)  # trailing block length
                
                if block_type == b'\x06\x00\x00\x00':  # Enhanced Packet Block
                    packets.append(block_data)
        elif magic == b'\xd4\xc3\xb2\xa1':
            # Classic PCAP (little-endian)
            f.seek(24)  # Skip global header
            while True:
                header = f.read(16)
                if len(header) < 16:
                    break
                ts_sec, ts_usec, incl_len, orig_len = struct.unpack('<IIII', header)
                packet_data = f.read(incl_len)
                packets.append(packet_data)
        else:
            print(f"Unknown PCAP format: {magic.hex()}")
    
    return packets

def parse_tcp_stream(packets):
    """Extract TCP payloads on port 9900"""
    tcp_payloads = []
    
    for pkt_data in packets:
        # Skip Ethernet header (14 bytes)
        if len(pkt_data) < 54:
            continue
        
        # Check if it's IP
        eth_type = struct.unpack('>H', pkt_data[12:14])[0]
        if eth_type != 0x0800:  # Not IPv4
            continue
        
        # Parse IP header
        ip_header = pkt_data[14:34]
        ip_proto = ip_header[9]
        if ip_proto != 6:  # Not TCP
            continue
        
        # Parse TCP header
        tcp_start = 14 + 20  # Ethernet + IP
        tcp_header = pkt_data[tcp_start:tcp_start+20]
        src_port = struct.unpack('>H', tcp_header[0:2])[0]
        dst_port = struct.unpack('>H', tcp_header[2:4])[0]
        
        if src_port != 9900 and dst_port != 9900:
            continue
        
        # Get TCP data offset
        data_offset = ((tcp_header[12] >> 4) & 0xF) * 4
        tcp_data_start = tcp_start + data_offset
        tcp_payload = pkt_data[tcp_data_start:]
        
        if len(tcp_payload) > 0:
            direction = "C->S" if dst_port == 9900 else "S->C"
            tcp_payloads.append((direction, tcp_payload))
    
    return tcp_payloads

def unmask_websocket(data, mask_key):
    """Unmask WebSocket payload"""
    unmasked = bytearray(data)
    for i in range(len(unmasked)):
        unmasked[i] ^= mask_key[i % 4]
    return bytes(unmasked)

def parse_websocket_frames(tcp_payloads):
    """Extract WebSocket binary frames"""
    ws_commands = []
    
    for direction, payload in tcp_payloads:
        offset = 0
        
        while offset < len(payload):
            if offset + 2 > len(payload):
                break
            
            # Check for WebSocket binary frame (0x82)
            if payload[offset] != 0x82:
                offset += 1
                continue
            
            # Parse payload length
            payload_len_byte = payload[offset + 1]
            masked = (payload_len_byte & 0x80) != 0
            payload_len = payload_len_byte & 0x7F
            
            header_len = 2
            if payload_len == 126:
                if offset + 4 > len(payload):
                    break
                payload_len = struct.unpack('>H', payload[offset+2:offset+4])[0]
                header_len = 4
            elif payload_len == 127:
                if offset + 10 > len(payload):
                    break
                payload_len = struct.unpack('>Q', payload[offset+2:offset+10])[0]
                header_len = 10
            
            mask_offset = offset + header_len
            if masked:
                mask_key = payload[mask_offset:mask_offset+4]
                data_offset = mask_offset + 4
            else:
                mask_key = None
                data_offset = mask_offset
            
            if data_offset + payload_len > len(payload):
                break
            
            ws_payload = payload[data_offset:data_offset+payload_len]
            
            if masked and mask_key:
                ws_payload = unmask_websocket(ws_payload, mask_key)
            
            if len(ws_payload) >= 10:
                ws_commands.append((direction, ws_payload))
            
            offset = data_offset + payload_len
    
    return ws_commands

def decode_command(data):
    """Decode WsPacket command"""
    # Simple varint decode for first few fields
    def read_varint(d, off):
        result = 0
        shift = 0
        while off < len(d):
            byte = d[off]
            result |= (byte & 0x7F) << shift
            off += 1
            if (byte & 0x80) == 0:
                break
            shift += 7
        return result, off
    
    offset = 0
    
    # Read packet length
    pkt_len, offset = read_varint(data, offset)
    
    # Parse WsPacket fields
    module_id = None
    cmd = None
    
    while offset < min(len(data), pkt_len + 10):
        if offset >= len(data):
            break
        
        tag, offset = read_varint(data, offset)
        field_num = tag >> 3
        wire_type = tag & 0x7
        
        if wire_type == 0:  # Varint
            value, offset = read_varint(data, offset)
            if field_num == 1:
                module_id = value
            elif field_num == 2:
                cmd = value
        elif wire_type == 2:  # Length-delimited
            length, offset = read_varint(data, offset)
            offset += length
        else:
            break
    
    return module_id, cmd

# Main
if len(sys.argv) < 2:
    print("Usage: parse_zwergii_pcap.py <pcap_file>")
    sys.exit(1)

filename = sys.argv[1]
print(f"Parsing {filename}...")

packets = read_pcapng(filename)
print(f"Read {len(packets)} packets")

tcp_payloads = parse_tcp_stream(packets)
print(f"Found {len(tcp_payloads)} TCP payloads on port 9900")

ws_commands = parse_websocket_frames(tcp_payloads)
print(f"Extracted {len(ws_commands)} WebSocket frames")
print()

# Filter for Module 20 commands (grid params)
print("="*80)
print("MODULE 20 COMMANDS (Grid Parameters)")
print("="*80)

mod20_count = 0
for direction, cmd_data in ws_commands:
    if direction == "C->S":
        module_id, cmd_id = decode_command(cmd_data)
        if module_id == 20:
            mod20_count += 1
            print(f"\nCommand #{mod20_count}:")
            print(f"  Direction: {direction}")
            print(f"  Module: {module_id}")
            print(f"  CMD: {cmd_id}")
            print(f"  Length: {len(cmd_data)} bytes")
            print(f"  Hex: {cmd_data.hex()}")
            
            # Check specific offsets for grid commands
            if len(cmd_data) >= 64 and cmd_id == 16703:
                print(f"  Offset[15]: 0x{cmd_data[15]:02x} ({'ROW' if cmd_data[15] == 0x9c else 'COL' if cmd_data[15] == 0x9d else 'UNKNOWN'})")
                if len(cmd_data) > 25:
                    print(f"  Offset[25]: 0x{cmd_data[25]:02x} (decimal={cmd_data[25]})")

if mod20_count == 0:
    print("\n❌ No Module 20 commands found!")
    print("\nSearching for ALL client commands...")
    print()
    
    for i, (direction, cmd_data) in enumerate(ws_commands):
        if direction == "C->S":
            module_id, cmd_id = decode_command(cmd_data)
            print(f"Command {i+1}: Module {module_id}, CMD {cmd_id}, Length {len(cmd_data)}")
