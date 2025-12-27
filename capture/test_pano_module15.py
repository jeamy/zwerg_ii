#!/usr/bin/env python3
"""Test Panorama mit Module 15, CMD 16703"""
import sys
sys.path.insert(0, '/media/data/programming/zwergII/capture')

import dwarf_proto_runtime as proto
from websocket import create_connection
import time

host = "10.42.0.209"
port = 9900

log_file = open('/media/data/programming/zwergII/capture/pano_test_mod15.log', 'w', buffering=1)

def log(msg):
    log_file.write(msg + '\n')
    log_file.flush()
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

def decode_selector_varint(hex_str):
    """Decode the special selector varint from hex string"""
    data = bytes.fromhex(hex_str.replace(' ', ''))
    val, _ = varint_decode(data)
    log(f"   Decoded selector: {hex_str} = {val} (0x{val:x})")
    return val

log("=== PANORAMA TEST - MODULE 15, CMD 16703 ===")
log(f"Host: {host}:{port}")
log(f"Requested: 3 rows x 4 cols = 12 images")
log("")

# Decode selectors aus Doku
log("Decoding selector varints from docs:")
row_selector = decode_selector_varint("9c 80 80 80 80 bc 81 07")
col_selector = decode_selector_varint("9d 80 80 80 80 bc 81 07")
log("")

try:
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log("Connected!")
    log("")
    
    # 1. Panorama UI Open
    log("STEP 1: Panorama UI Open (Module 14, CMD 16402)")
    pkt = proto.WsPacket(module_id=14, cmd=16402, type=0, data=bytes.fromhex("0807"))
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    time.sleep(1)
    log("")
    
    # 2. Set Rows via Module 15, CMD 16703
    log("STEP 2: Set Rows=3 (Module 15, CMD 16703)")
    log(f"   Selector: {row_selector} (0x{row_selector:x})")
    log(f"   Value: 3")
    
    # Build payload: field 1 (tag 0x08) = selector, field 2 (tag 0x10) = value
    payload_rows = bytearray()
    payload_rows.append(0x08)  # tag for field 1 (varint)
    payload_rows.extend(varint_encode(row_selector))
    payload_rows.append(0x10)  # tag for field 2 (varint)
    payload_rows.extend(varint_encode(3))  # rows=3
    
    log(f"   Payload hex: {payload_rows.hex()}")
    
    pkt = proto.WsPacket(module_id=15, cmd=16703, type=0, data=bytes(payload_rows))
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    time.sleep(1)
    log("")
    
    # 3. Set Cols via Module 15, CMD 16703
    log("STEP 3: Set Cols=4 (Module 15, CMD 16703)")
    log(f"   Selector: {col_selector} (0x{col_selector:x})")
    log(f"   Value: 4")
    
    payload_cols = bytearray()
    payload_cols.append(0x08)  # tag for field 1
    payload_cols.extend(varint_encode(col_selector))
    payload_cols.append(0x10)  # tag for field 2
    payload_cols.extend(varint_encode(4))  # cols=4
    
    log(f"   Payload hex: {payload_cols.hex()}")
    
    pkt = proto.WsPacket(module_id=15, cmd=16703, type=0, data=bytes(payload_cols))
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    time.sleep(1)
    log("")
    
    # 4. Start Panorama
    log("STEP 4: Start Panorama (Module 10, CMD 15500)")
    req_start = proto.ReqStartPanoramaByGrid()
    payload_start = req_start.SerializeToString()
    pkt = proto.WsPacket(module_id=10, cmd=15500, type=0, data=payload_start)
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    log("")
    
    # 5. Monitor progress
    log("STEP 5: Monitoring panorama progress...")
    ws.settimeout(120.0)
    start = time.time()
    
    while time.time() - start < 120:
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
                    log(f"   Progress: {current}/{total}")
                    if current >= total:
                        log("")
                        log("="*60)
                        log("PANORAMA COMPLETE!")
                        log(f"Requested: 3 x 4 = 12 images")
                        log(f"Actual:    {total} images")
                        log("")
                        if total == 12:
                            log("✓ SUCCESS - Grid settings applied correctly!")
                        else:
                            log("✗ FAILURE - DWARF ignored grid settings!")
                            if total == 25:
                                log("  → DWARF used default 5x5 grid")
                        log("="*60)
                        break
        except Exception as e:
            log(f"Error/Timeout: {e}")
            break
    
    ws.close()
    log("")
    log("Test complete.")
    log_file.close()
    
except Exception as e:
    log(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
    log_file.close()
