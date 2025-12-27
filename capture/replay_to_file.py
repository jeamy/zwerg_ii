#!/usr/bin/env python3
import sys
import time
import os
from websocket import create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

# Write to file from start
logfile = open('/media/data/programming/zwergII/capture/REPLAY_RESULT.txt', 'w', buffering=1)

def log(msg):
    logfile.write(msg + '\n')
    logfile.flush()

host = "10.42.0.209"
port = 9900

# EXACT Android payloads from ctrl_20251226_1.pcapng
PAYLOADS = [
    "080110141801200e289280013a020807422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200f28bf82013a0c089c8080808080bc81071003422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200f28bf82013a0c089d8080808080bc81071004422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200a288c79422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
]

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

def send_masked(ws, payload_hex):
    payload = bytes.fromhex(payload_hex)
    mask = os.urandom(4)
    masked = bytearray(payload)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    frame = bytearray([0x82])
    if len(payload) < 126:
        frame.append(0x80 | len(payload))
    else:
        frame.append(0x80 | 126)
        frame.extend(len(payload).to_bytes(2, 'big'))
    frame.extend(mask)
    frame.extend(masked)
    
    ws.send(bytes(frame), opcode=0x2)
    return len(payload)

try:
    log("="*80)
    log("EXACT ANDROID REPLAY - ctrl_20251226_1.pcapng")
    log(f"Target: {host}:{port}")
    log(f"Goal: 3 rows x 4 cols = 12 images")
    log("="*80)
    log("")
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log("Connected!")
    log("")
    
    log("Step 1: Panorama UI Open")
    send_masked(ws, PAYLOADS[0])
    log("  Sent")
    time.sleep(2.0)
    
    log("Step 2: ROW=3")
    send_masked(ws, PAYLOADS[1])
    log("  Sent")
    time.sleep(2.0)
    
    log("Step 3: COL=4")
    send_masked(ws, PAYLOADS[2])
    log("  Sent")
    time.sleep(2.0)
    
    log("Step 4: Start Panorama")
    send_masked(ws, PAYLOADS[3])
    log("  Sent")
    log("")
    
    log("Monitoring progress (120s timeout)...")
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
                    log(f"  {current}/{total}")
                    if current >= total:
                        log("")
                        log("="*80)
                        log("COMPLETE")
                        log(f"Expected: 12 images (3x4)")
                        log(f"Got:      {total} images")
                        if total == 12:
                            log("")
                            log("SUCCESS - Exact Android replay works!")
                        else:
                            log("")
                            log(f"FAILURE - Got {total} instead of 12")
                        log("="*80)
                        break
        except Exception as e:
            log(f"Error: {e}")
            break
    
    ws.close()
    log("\nDone.")
    logfile.close()
    
except Exception as e:
    log(f"ERROR: {e}")
    import traceback
    traceback.print_exc(file=logfile)
    logfile.close()
