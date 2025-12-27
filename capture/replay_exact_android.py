#!/usr/bin/env python3
"""
Replay EXAKTER Android Payloads aus ctrl_20251226_1.pcapng
Commands 1-5: UI Open, ROW=3, COL=4, Start
"""
import sys
import time
import os
from websocket import create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

host = "10.42.0.209"
port = 9900

# EXAKTE Payloads aus Android PCAP (ctrl_20251226_1_commands.json)
ANDROID_PAYLOADS = [
    # Command 1: Panorama UI Open (54 bytes)
    "080110141801200e289280013a020807422432303939643762392d323537612d343166632d613161622d376535316165326630333030",
    
    # Command 3: ROW=3 (64 bytes, selector 0x9c at offset 15)
    "080110141801200f28bf82013a0c089c8080808080bc81071003422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    
    # Command 4: COL=4 (64 bytes, selector 0x9d at offset 15)
    "080110141801200f28bf82013a0c089d8080808080bc81071004422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    
    # Command 5: Panorama START (49 bytes)
    "080110141801200a288c79422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
]

def log(msg):
    sys.stdout.write(msg + '\n')
    sys.stdout.flush()

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
    """Send payload as masked WebSocket binary frame"""
    payload = bytes.fromhex(payload_hex)
    
    # Mask
    mask = os.urandom(4)
    masked = bytearray(payload)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    # Build frame
    frame = bytearray([0x82])  # FIN + Binary
    
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
    log("EXACT ANDROID PAYLOAD REPLAY")
    log(f"Host: {host}:{port}")
    log(f"Goal: 3 rows x 4 cols = 12 images")
    log("="*80)
    log("")
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log("✓ Connected\n")
    
    # Send commands with delays
    log("Command 1: Panorama UI Open")
    sent = send_masked(ws, ANDROID_PAYLOADS[0])
    log(f"  Sent {sent} bytes")
    time.sleep(2.0)
    log("")
    
    log("Command 2: Set ROW=3")
    payload = bytes.fromhex(ANDROID_PAYLOADS[1])
    log(f"  Verify: offset[15]=0x{payload[15]:02x} (expect 0x9c)")
    log(f"  Verify: offset[25]=0x{payload[25]:02x} (expect 0x03)")
    sent = send_masked(ws, ANDROID_PAYLOADS[1])
    log(f"  Sent {sent} bytes")
    time.sleep(2.0)
    log("")
    
    log("Command 3: Set COL=4")
    payload = bytes.fromhex(ANDROID_PAYLOADS[2])
    log(f"  Verify: offset[15]=0x{payload[15]:02x} (expect 0x9d)")
    log(f"  Verify: offset[25]=0x{payload[25]:02x} (expect 0x04)")
    sent = send_masked(ws, ANDROID_PAYLOADS[2])
    log(f"  Sent {sent} bytes")
    time.sleep(2.0)
    log("")
    
    log("Command 4: Start Panorama")
    sent = send_masked(ws, ANDROID_PAYLOADS[3])
    log(f"  Sent {sent} bytes")
    log("")
    
    # Monitor progress
    log("Monitoring panorama progress...")
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
                    fn = tag >> 3
                    wt = tag & 0x7
                    if wt == 0:
                        val, off2 = varint_decode(data, off2)
                        if fn == 1:
                            current = val
                        elif fn == 2:
                            total = val
                
                if current and total:
                    log(f"  Progress: {current}/{total}")
                    if current >= total:
                        log("")
                        log("="*80)
                        log("PANORAMA COMPLETE")
                        log(f"Requested: 3 x 4 = 12 images")
                        log(f"Actual:    {total} images")
                        log("")
                        if total == 12:
                            log("✓✓✓ SUCCESS - Exact Android replay works! ✓✓✓")
                        else:
                            log(f"✗ FAILURE - Got {total} instead of 12")
                            if total == 25:
                                log("  → Still defaulting to 5x5")
                        log("="*80)
                        break
        except Exception as e:
            log(f"Error/Timeout: {e}")
            break
    
    ws.close()
    log("\nDone.")
    
except Exception as e:
    log(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
