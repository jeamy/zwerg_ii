#!/usr/bin/env python3
"""
Replay WITH response handling - read responses after each command
"""
import sys
import time
import os
from websocket import create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

logfile = open('/media/data/programming/zwergII/capture/RESPONSE_REPLAY.txt', 'w', buffering=1)

def log(msg):
    logfile.write(msg + '\n')
    logfile.flush()

host = "10.42.0.209"
port = 9900

# Commands from ctrl_20251226_113958.pcapng
INIT_COMMANDS = [
    "080110141801200428c8653a0f0882ccb9ca0611000000000000f03f422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200428c9653a0f0a0d4575726f70652f5669656e6e61422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200d28e67d422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200e289480013a041a020801422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200328a056422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    "080110141801200128c24e3a020801422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
]

PANORAMA_COMMANDS = [
    "080110141801200e289280013a020807422432303939643762392d323537612d343166632d613161622d376535316165326630303030",  # UI Open
    "080110141801200f28bf82013a0c089c8080808080bc81071003422432303939643762392d323537612d343166632d613161622d376535316165326630303030",  # ROW=3
    "080110141801200f28bf82013a0c089d8080808080bc81071004422432303939643762392d323537612d343166632d613161622d376535316165326630303030",  # COL=4
    "080110141801200a288c79422432303939643762392d323537612d343166632d613161622d376535316165326630303030",  # START
]

# Expected response counts from analysis
EXPECTED_RESPONSES = {
    'init': [1, 1, 4, 0, 1, 2],
    'panorama': [4, 2, 2, 15]  # UI Open, ROW, COL, START
}

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

def read_responses(ws, expected_count, timeout=2.0):
    """Read expected number of responses"""
    responses = []
    ws.settimeout(timeout)
    
    for i in range(expected_count):
        try:
            raw = ws.recv()
            if raw:
                responses.append(raw)
        except Exception as e:
            log(f"    Warning: Expected {expected_count} responses, got {len(responses)} (timeout)")
            break
    
    return responses

try:
    log("="*80)
    log("REPLAY WITH RESPONSE HANDLING")
    log(f"Target: {host}:{port}")
    log(f"Goal: 3 rows x 4 cols = 12 images")
    log("="*80)
    log("")
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log("✓ Connected")
    log("")
    
    # PHASE 1: Init
    log("PHASE 1: Init (6 commands with response handling)")
    for i, cmd_hex in enumerate(INIT_COMMANDS):
        expected = EXPECTED_RESPONSES['init'][i]
        log(f"  Init {i+1}/6 (expect {expected} response(s))...")
        send_masked(ws, cmd_hex)
        
        if expected > 0:
            responses = read_responses(ws, expected)
            log(f"    Got {len(responses)} response(s)")
        
        time.sleep(0.3)
    
    log("  ✓ Init complete")
    log("")
    
    # PHASE 2: Panorama
    log("PHASE 2: Panorama Sequence")
    
    cmd_names = ["UI Open", "ROW=3", "COL=4", "START"]
    
    for i, cmd_hex in enumerate(PANORAMA_COMMANDS[:3]):  # Skip START for now
        expected = EXPECTED_RESPONSES['panorama'][i]
        log(f"  {cmd_names[i]} (expect {expected} response(s))...")
        send_masked(ws, cmd_hex)
        
        responses = read_responses(ws, expected)
        log(f"    Got {len(responses)} response(s)")
        
        time.sleep(0.5)
    
    log("")
    log("  Sending Panorama START...")
    send_masked(ws, PANORAMA_COMMANDS[3])
    
    # START command sends progress notifications - monitor them
    log("")
    log("PHASE 3: Monitor Progress")
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
                    log(f"  {current}/{total}")
                    if current >= total:
                        log("")
                        log("="*80)
                        log("PANORAMA COMPLETE!")
                        log(f"Expected: 3 x 4 = 12 images")
                        log(f"Got:      {total} images")
                        log("")
                        if total == 12:
                            log("✓✓✓ SUCCESS ✓✓✓")
                            log("")
                            log("WORKING PROTOCOL:")
                            log("1. Send init commands + read responses")
                            log("2. Send Panorama UI Open + read 4 responses")
                            log("3. Send ROW setting + read 2 responses")
                            log("4. Send COL setting + read 2 responses")
                            log("5. Send START + monitor progress notifications")
                        else:
                            log(f"✗ Got {total} instead of 12")
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
