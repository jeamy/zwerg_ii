#!/usr/bin/env python3
"""
MINIMALER Test - sende Commands und WARTE auf jede Response
Basierend auf dump.md Analyse aber mit Response-Handling
"""
import sys
import time
import os
from websocket import WebSocket, create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

host = "10.42.0.209"
port = 9900

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

def send_and_wait(ws, payload_bytes, wait_for_response=True, timeout=3.0):
    """Send command and optionally wait for response"""
    # Mask payload
    mask = os.urandom(4)
    masked = bytearray(payload_bytes)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    # Build WebSocket frame
    frame = bytearray([0x82])  # FIN + Binary
    
    if len(payload_bytes) < 126:
        frame.append(0x80 | len(payload_bytes))
    else:
        frame.append(0x80 | 126)
        frame.extend(len(payload_bytes).to_bytes(2, 'big'))
    
    frame.extend(mask)
    frame.extend(masked)
    
    print(f">>> Sending {len(payload_bytes)} bytes...")
    ws.send(bytes(frame), opcode=0x2)
    
    if not wait_for_response:
        return None
    
    # Wait for response
    print(f"    Waiting for response (timeout={timeout}s)...")
    ws.settimeout(timeout)
    start = time.time()
    
    while time.time() - start < timeout:
        try:
            raw = ws.recv()
            if not raw:
                continue
            
            # Decode
            pkt_len, off = varint_decode(raw)
            pkt_data = raw[off:off+pkt_len]
            
            resp = proto.WsPacket()
            resp.ParseFromString(pkt_data)
            
            print(f"<<< Response: module={resp.module_id}, cmd={resp.cmd}, type={resp.type}")
            
            # Skip notifications (type=0)
            if resp.type != 0:
                return resp
        except Exception as e:
            print(f"    Timeout or error: {e}")
            return None
    
    return None

def build_grid_cmd(selector, value):
    """Build 64-byte grid command from dump.md template"""
    template = bytearray(bytes.fromhex(
        "080110141801200f28bf8201"
        "3208011009120418061801201c28013001390000000000001c40"
        "419c808080e05e48014a0c089b8080808aa78a0210022001"
        "610000000000001440"
    ))
    template[15] = selector
    template[25] = value
    return bytes(template)

try:
    print("="*80)
    print("MINIMAL PANORAMA TEST WITH RESPONSE WAITING")
    print(f"Host: {host}:{port}")
    print("="*80)
    print()
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    print("✓ Connected\n")
    
    # Panorama UI Open
    print("STEP 1: Panorama UI Open (Module 14, CMD 16402)")
    pkt = proto.WsPacket(module_id=14, cmd=16402, type=0, data=bytes.fromhex("0807"))
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    send_and_wait(ws, msg, wait_for_response=False)
    time.sleep(2.0)  # Wait longer
    print()
    
    # Set Rows
    print("STEP 2: Set Rows = 3")
    row_cmd = build_grid_cmd(0x9c, 3)
    send_and_wait(ws, row_cmd, wait_for_response=False)
    time.sleep(2.0)
    print()
    
    # Set Cols
    print("STEP 3: Set Cols = 4")
    col_cmd = build_grid_cmd(0x9d, 4)
    send_and_wait(ws, col_cmd, wait_for_response=False)
    time.sleep(2.0)
    print()
    
    # Start Panorama
    print("STEP 4: Start Panorama (Module 10, CMD 15500)")
    start_pkt = proto.WsPacket(module_id=10, cmd=15500, type=0, data=b"")
    start_msg = varint_encode(len(start_pkt.SerializeToString())) + start_pkt.SerializeToString()
    send_and_wait(ws, start_msg, wait_for_response=False)
    print()
    
    # Monitor progress
    print("STEP 5: Monitor Progress...")
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
            
            # Progress notification
            if resp.module_id == 10 and resp.cmd == 15219:
                data = resp.data
                current = None
                total = None
                off2 = 0
                while off2 < len(data):
                    tag, off2 = varint_decode(data, off2)
                    fn = tag >> 3
                    wt = tag & 0x7
                    if wt == 0:
                        val, off2 = varint_decode(data, off2)
                        if fn == 1:
                            current = val
                        elif fn == 2:
                            total = val
                
                if current and total:
                    print(f"  Progress: {current}/{total}")
                    if current >= total:
                        print()
                        print("="*80)
                        print("PANORAMA COMPLETE")
                        print(f"Requested: 3 x 4 = 12")
                        print(f"Actual: {total}")
                        if total == 12:
                            print("✓✓✓ SUCCESS ✓✓✓")
                        else:
                            print(f"✗ FAILURE (got {total} instead of 12)")
                        print("="*80)
                        break
        except Exception as e:
            print(f"Error: {e}")
            break
    
    ws.close()
    print("\nDone.")
    
except Exception as e:
    print(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
