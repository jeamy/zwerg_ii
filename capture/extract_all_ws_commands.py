#!/usr/bin/env python3
"""
Extrahiere ALLE WebSocket Binary Commands aus PCAP
Unmask Client->Server Payloads und speichere sie zum Replay
"""
import struct
import json


def read_varint(buf, offset):
    shift = 0
    value = 0
    while offset < len(buf):
        b = buf[offset]
        offset += 1
        value |= (b & 0x7F) << shift
        if (b & 0x80) == 0:
            return value, offset
        shift += 7
        if shift > 70:
            break
    return None, offset


def parse_protobuf_fields(buf):
    offset = 0
    fields = {}
    while offset < len(buf):
        key, offset = read_varint(buf, offset)
        if key is None:
            break
        field_num = key >> 3
        wire_type = key & 0x07
        if wire_type == 0:
            v, offset = read_varint(buf, offset)
            if v is None:
                break
            fields.setdefault(field_num, []).append(v)
        elif wire_type == 2:
            ln, offset = read_varint(buf, offset)
            if ln is None:
                break
            end = offset + ln
            if end > len(buf):
                break
            fields.setdefault(field_num, []).append(buf[offset:end])
            offset = end
        else:
            break
    return fields


def decode_ws_packet(ws_payload):
    f = parse_protobuf_fields(ws_payload)
    module_id = f.get(4, [None])[0]
    cmd = f.get(5, [None])[0]
    msg_type = f.get(6, [None])[0]
    data = f.get(7, [b''])[0] if 7 in f else b''
    return module_id, cmd, msg_type, data


def summarize_protobuf_fields(fields):
    out = {}
    for k, vals in fields.items():
        key = str(k)
        out_vals = []
        for v in vals:
            if isinstance(v, (bytes, bytearray)):
                out_vals.append({
                    'bytes_len': len(v),
                    'bytes_hex_prefix': v[:24].hex(),
                })
            else:
                out_vals.append(v)
        out[key] = out_vals
    return out


def extract_ws_frames_from_stream(buf):
    """Parse as many WebSocket frames from buf as possible.

    Returns: (frames, remaining_bytes)
    frames is a list of (opcode, payload_bytes)
    """
    frames = []
    offset = 0

    while True:
        if offset + 2 > len(buf):
            break
        b1 = buf[offset]
        b2 = buf[offset + 1]
        opcode = b1 & 0x0F
        masked = (b2 & 0x80) != 0
        ln = b2 & 0x7F
        hdr = 2

        if ln == 126:
            if offset + hdr + 2 > len(buf):
                break
            ln = struct.unpack('>H', buf[offset + hdr:offset + hdr + 2])[0]
            hdr += 2
        elif ln == 127:
            if offset + hdr + 8 > len(buf):
                break
            ln = struct.unpack('>Q', buf[offset + hdr:offset + hdr + 8])[0]
            hdr += 8

        mask_len = 4 if masked else 0
        total = hdr + mask_len + ln
        if offset + total > len(buf):
            break

        i = offset + hdr
        mask = None
        if masked:
            mask = buf[i:i + 4]
            i += 4

        payload = bytearray(buf[i:i + ln])
        if mask:
            for j in range(len(payload)):
                payload[j] ^= mask[j % 4]

        frames.append((opcode, bytes(payload)))
        offset += total

    return frames, buf[offset:]

def read_pcapng(filename):
    """Read pcapng OR classic pcap file and extract packets"""
    packets = []

    with open(filename, 'rb') as f:
        data = f.read()

    if data[:4] == b'\x0a\x0d\x0d\x0a':
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

    if data[:4] in (b'\xd4\xc3\xb2\xa1', b'\xa1\xb2\xc3\xd4'):
        endian = '<' if data[:4] == b'\xd4\xc3\xb2\xa1' else '>'
        offset = 24
        while offset + 16 <= len(data):
            ts_sec, ts_usec, incl_len, orig_len = struct.unpack(
                endian + 'IIII', data[offset:offset+16]
            )
            offset += 16
            if incl_len == 0 or offset + incl_len > len(data):
                break
            packet_data = data[offset:offset+incl_len]
            packets.append(packet_data)
            offset += incl_len
        return packets

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

    # Commands we want to further introspect by decoding the inner protobuf fields.
    # This keeps JSON size manageable while still allowing analysis of stacking.
    interesting_cmds = {
        11005, 11006, 11010, 11016, 11017,
        15208, 15209, 15236, 15237,
        10007, 10009, 10011, 10013,
        12002, 12004, 12005, 12006,
        15213, 15214,
    }

    # Very lightweight TCP reassembly: accumulate per-direction stream buffers.
    # This is not a full TCP reassembler, but it handles common frame splitting.
    stream_buffers = {}

    for i, tcp_pkt in enumerate(tcp_payloads):
        direction = tcp_pkt['direction']
        payload = tcp_pkt['payload']

        stream_key = (direction, tcp_pkt.get('src'), tcp_pkt.get('dst'))
        buf = stream_buffers.get(stream_key, b'') + payload
        frames, remainder = extract_ws_frames_from_stream(buf)
        stream_buffers[stream_key] = remainder

        for opcode, ws_payload in frames:
            # Binary frame (opcode 2)
            if opcode != 2:
                continue

            module_id, cmd, msg_type, data = decode_ws_packet(ws_payload)
            entry = {
                'index': i,
                'direction': direction,
                'opcode': opcode,
                'payload_hex': ws_payload.hex(),
                'payload_len': len(ws_payload),
                'wsp_module_id': module_id,
                'wsp_cmd': cmd,
                'wsp_type': msg_type,
                'wsp_data_len': len(data) if data is not None else 0,
                'wsp_data_hex': data.hex() if data is not None else "",
            }

            if cmd in interesting_cmds and data:
                entry['wsp_data_fields'] = summarize_protobuf_fields(parse_protobuf_fields(data))

            commands.append(entry)
    
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
