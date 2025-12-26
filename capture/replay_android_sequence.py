#!/usr/bin/env python3
"""
Replay der EXAKTEN Android-Sequenz aus dump.md Analyse
Vollständige Sequenz inkl. Panorama-Screen öffnen, Grid ändern, Start
"""
import sys
import time
import os
from websocket import create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

host = "10.42.0.209"
port = 9900

log = open('/media/data/programming/zwergII/capture/replay_log.txt', 'w', buffering=1)

def log_msg(msg):
    log.write(msg + '\n')
    log.flush()
    print(msg, flush=True)

def varint_encode(n):
    result = bytearray()
    while n > 0x7F:
        result.append((n & 0x7F) | 0x80)
        n >>= 7
    result.append(n & 0x7F)
    return bytes(result)

def varint_decode(data, offset=0):
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

def send_raw_masked(ws, payload_bytes):
    """Send raw payload as masked WebSocket binary frame"""
    mask = os.urandom(4)
    masked = bytearray(payload_bytes)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    frame = bytearray()
    frame.append(0x82)  # FIN + Binary
    
    if len(payload_bytes) < 126:
        frame.append(0x80 | len(payload_bytes))
    elif len(payload_bytes) < 65536:
        frame.append(0x80 | 126)
        frame.extend(len(payload_bytes).to_bytes(2, 'big'))
    else:
        frame.append(0x80 | 127)
        frame.extend(len(payload_bytes).to_bytes(8, 'big'))
    
    frame.extend(mask)
    frame.extend(masked)
    
    ws.send(bytes(frame), opcode=0x2)
    return len(frame)

def build_grid_command(selector_byte, value):
    """
    Baue 64-byte Panorama Grid Command
    Template aus dump.md, Offset 15 = selector, Offset 25 = value
    """
    # Base template aus dump.md (71 bytes nach correction)
    template = bytearray(bytes.fromhex(
        "080110141801200f28bf8201"
        "3208011009120418061801201c28013001390000000000001c40"
        "419c808080e05e48014a0c089b8080808aa78a0210022001"
        "610000000000001440"
    ))
    
    # Patch selector und value
    template[15] = selector_byte
    template[25] = value
    
    return bytes(template)

try:
    log_msg("="*80)
    log_msg("ANDROID SEQUENCE REPLAY")
    log_msg(f"Host: {host}:{port}")
    log_msg(f"Goal: Replicate full Android sequence for 3x4 panorama")
    log_msg("="*80)
    log_msg("")
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log_msg("✓ Connected")
    log_msg("")
    
    time.sleep(0.5)
    
    # PHASE 1: Panorama UI öffnen (Module 14, CMD 16402)
    log_msg("PHASE 1: Open Panorama UI (Module 14, CMD 16402)")
    pkt = proto.WsPacket(module_id=14, cmd=16402, type=0, data=bytes.fromhex("0807"))
    pkt_bytes = pkt.SerializeToString()
    msg = varint_encode(len(pkt_bytes)) + pkt_bytes
    sent = send_raw_masked(ws, msg)
    log_msg(f"  Sent {sent} bytes")
    time.sleep(1.0)
    log_msg("")
    
    # PHASE 2: Grid-Einstellungen setzen (aus dump.md: abwechselnd row/col)
    # Wir wollen 3 rows, 4 cols
    
    log_msg("PHASE 2: Set Grid Parameters")
    log_msg("  Step 2a: Set ROWS = 3")
    row_cmd = build_grid_command(0x9c, 3)  # 0x9c = row selector
    sent = send_raw_masked(ws, row_cmd)
    log_msg(f"    Sent {sent} bytes (selector=0x9c, value=3)")
    time.sleep(0.5)
    
    log_msg("  Step 2b: Set COLS = 4")
    col_cmd = build_grid_command(0x9d, 4)  # 0x9d = col selector
    sent = send_raw_masked(ws, col_cmd)
    log_msg(f"    Sent {sent} bytes (selector=0x9d, value=4)")
    time.sleep(0.5)
    log_msg("")
    
    # PHASE 3: Panorama starten (Module 10, CMD 15500)
    log_msg("PHASE 3: Start Panorama (Module 10, CMD 15500)")
    start_pkt = proto.WsPacket(module_id=10, cmd=15500, type=0, data=b"")
    start_bytes = start_pkt.SerializeToString()
    start_msg = varint_encode(len(start_bytes)) + start_bytes
    sent = send_raw_masked(ws, start_msg)
    log_msg(f"  Sent {sent} bytes")
    time.sleep(1.0)
    log_msg("")
    
    # PHASE 4: Monitor panorama progress
    log_msg("PHASE 4: Monitor Panorama Progress")
    ws.settimeout(120.0)
    start_time = time.time()
    
    while time.time() - start_time < 120:
        try:
            raw = ws.recv()
            if not raw:
                continue
            
            pkt_len, off = varint_decode(raw)
            pkt_data = raw[off:off+pkt_len]
            
            resp = proto.WsPacket()
            resp.ParseFromString(pkt_data)
            
            # Panorama progress: module=10, cmd=15219
            if resp.module_id == 10 and resp.cmd == 15219:
                data = resp.data
                current = None
                total = None
                off2 = 0
                while off2 < len(data):
                    tag, off2 = varint_decode(data, off2)
                    field_num = tag >> 3
                    wire_type = tag & 0x7
                    if wire_type == 0:
                        value, off2 = varint_decode(data, off2)
                        if field_num == 1:
                            current = value
                        elif field_num == 2:
                            total = value
                
                if current and total:
                    log_msg(f"  Progress: {current}/{total}")
                    if current >= total:
                        log_msg("")
                        log_msg("="*80)
                        log_msg("PANORAMA COMPLETE")
                        log_msg(f"Requested: 3 x 4 = 12 images")
                        log_msg(f"Actual:    {total} images")
                        log_msg("")
                        if total == 12:
                            log_msg("✓✓✓ SUCCESS - Android sequence replay works! ✓✓✓")
                        else:
                            log_msg(f"✗ FAILURE - Got {total} instead of 12")
                            if total == 25:
                                log_msg("  → Still using default 5x5 grid")
                        log_msg("="*80)
                        break
        except Exception as e:
            log_msg(f"  Error/Timeout: {e}")
            break
    
    ws.close()
    log_msg("")
    log_msg("Replay complete.")
    log.close()
    
except Exception as e:
    log_msg(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
    log.close()
