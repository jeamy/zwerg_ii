#!/usr/bin/env python3
import sys
sys.path.insert(0, '/media/data/programming/zwergII/capture')

log_file = open('/media/data/programming/zwergII/capture/pano_test_live.log', 'w', buffering=1)

def log(msg):
    log_file.write(msg + '\n')
    log_file.flush()
    print(msg, flush=True)

log("Starting test...")

import dwarf_proto_runtime as proto
from websocket import create_connection
import time

host = "10.42.0.209"
port = 9900

log(f"Connecting to {host}:{port}...")

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

try:
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log("Connected!")
    
    # Send UI Open
    log("\n1. Sending Panorama UI Open (14, 16402)...")
    pkt = proto.WsPacket(module_id=14, cmd=16402, type=0, data=bytes.fromhex("0807"))
    payload = pkt.SerializeToString()
    msg = varint_encode(len(payload)) + payload
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    time.sleep(1)
    
    # Try to read any responses
    log("   Checking for responses...")
    ws.settimeout(2.0)
    try:
        for i in range(5):
            raw = ws.recv()
            if raw:
                pkt_len, off = varint_decode(raw)
                pkt_data = raw[off:off+pkt_len]
                resp = proto.WsPacket()
                resp.ParseFromString(pkt_data)
                print(f"   RX: mod={resp.module_id}, cmd={resp.cmd}, type={resp.type}", flush=True)
    except Exception as e:
        log(f"   No more responses: {e}")
    
    # Send Rows (3)
    log("\n2. Sending Rows=3 (1, 10037, ID=6)...")
    param = proto.CommonParam(id=6, mode_index=1, continue_value=3.0)
    req = proto.ReqSetFeatureParams(param=param)
    payload = req.SerializeToString()
    log(f"   Payload hex: {payload.hex()}")
    pkt = proto.WsPacket(module_id=1, cmd=10037, type=0, data=payload)
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    time.sleep(1)
    
    ws.settimeout(2.0)
    try:
        for i in range(5):
            raw = ws.recv()
            if raw:
                pkt_len, off = varint_decode(raw)
                pkt_data = raw[off:off+pkt_len]
                resp = proto.WsPacket()
                resp.ParseFromString(pkt_data)
                log(f"   RX: mod={resp.module_id}, cmd={resp.cmd}, type={resp.type}")
                if resp.data:
                    try:
                        com = proto.ComResponse()
                        com.ParseFromString(resp.data)
                        log(f"   ComResponse.code = {com.code}")
                    except:
                        pass
    except Exception as e:
        log(f"   No more responses: {e}")
    
    # Send Cols (4)
    log("\n3. Sending Cols=4 (1, 10037, ID=7)...")
    param = proto.CommonParam(id=7, mode_index=1, continue_value=4.0)
    req = proto.ReqSetFeatureParams(param=param)
    payload = req.SerializeToString()
    log(f"   Payload hex: {payload.hex()}")
    pkt = proto.WsPacket(module_id=1, cmd=10037, type=0, data=payload)
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    time.sleep(1)
    
    ws.settimeout(2.0)
    try:
        for i in range(5):
            raw = ws.recv()
            if raw:
                pkt_len, off = varint_decode(raw)
                pkt_data = raw[off:off+pkt_len]
                resp = proto.WsPacket()
                resp.ParseFromString(pkt_data)
                log(f"   RX: mod={resp.module_id}, cmd={resp.cmd}, type={resp.type}")
                if resp.data:
                    try:
                        com = proto.ComResponse()
                        com.ParseFromString(resp.data)
                        log(f"   ComResponse.code = {com.code}")
                    except:
                        pass
    except Exception as e:
        log(f"   No more responses: {e}")
    
    # Start Panorama
    log("\n4. Sending Start Panorama (10, 15500)...")
    req_start = proto.ReqStartPanoramaByGrid()
    payload = req_start.SerializeToString()
    log(f"   Payload: {len(payload)} bytes")
    pkt = proto.WsPacket(module_id=10, cmd=15500, type=0, data=payload)
    msg = varint_encode(len(pkt.SerializeToString())) + pkt.SerializeToString()
    ws.send(msg, opcode=0x2)
    log(f"   Sent {len(msg)} bytes")
    
    # Monitor for notifications
    log("\n5. Monitoring panorama progress (60s)...")
    ws.settimeout(60.0)
    start = time.time()
    
    while time.time() - start < 60:
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
                        log(f"\n✓ PANORAMA COMPLETE!")
                        log(f"   Requested: 3x4 = 12 images")
                        log(f"   Actual: {total} images")
                        if total == 12:
                            log(f"   ✓ SUCCESS!")
                        else:
                            log(f"   ✗ FAILURE - Grid ignored!")
                        break
        except Exception as e:
            log(f"   Error/Timeout: {e}")
            break
    
    ws.close()
    log("\nTest complete.")
    log_file.close()
    
except Exception as e:
    log(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
    log_file.close()
