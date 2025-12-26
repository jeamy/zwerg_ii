#!/usr/bin/env python3
"""
Replay COMPLETE Android session with ALL init commands
Based on ctrl_20251226_113958.pcapng
"""
import sys
import time
import os
from websocket import create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto

logfile = open('/media/data/programming/zwergII/capture/FULL_SESSION_RESULT.txt', 'w', buffering=1)

def log(msg):
    logfile.write(msg + '\n')
    logfile.flush()

host = "10.42.0.209"
port = 9900

# COMPLETE command sequence from ctrl_20251226_113958.pcapng
# Commands 1-6: Init sequence
# Command 7: Panorama UI Open
# Commands 8-9: ROW=5, COL=5
# Command 10: Panorama START

INIT_COMMANDS = [
    # Command 1: System setup
    "080110141801200428c8653a0f0882ccb9ca0611000000000000f03f422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 2: Timezone
    "080110141801200428c9653a0f0a0d4575726f70652f5669656e6e61422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 3: Mode init
    "080110141801200d28e67d422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 4: Camera init
    "080110141801200e289480013a041a020801422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 5: Setup
    "080110141801200328a056422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 6: Camera open
    "080110141801200128c24e3a020801422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
]

PANORAMA_COMMANDS = [
    # Command 7: Panorama UI Open
    "080110141801200e289280013a020807422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 8: ROW=3 (modified from ROW=5)
    "080110141801200f28bf82013a0c089c8080808080bc81071003422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 9: COL=4 (modified from COL=5)
    "080110141801200f28bf82013a0c089d8080808080bc81071004422432303939643762392d323537612d343166632d613161622d376535316165326630303030",
    # Command 10: Panorama START
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
    log("COMPLETE SESSION REPLAY - WITH INIT COMMANDS")
    log(f"Target: {host}:{port}")
    log(f"Goal: 3 rows x 4 cols = 12 images")
    log("="*80)
    log("")
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    log("✓ Connected")
    log("")
    
    # PHASE 1: Init sequence (Commands 1-6)
    log("PHASE 1: Init Sequence (6 commands)")
    for i, cmd_hex in enumerate(INIT_COMMANDS):
        log(f"  Init {i+1}/6...")
        send_masked(ws, cmd_hex)
        time.sleep(0.5)
    log("  ✓ Init complete")
    log("")
    
    # PHASE 2: Panorama sequence
    log("PHASE 2: Panorama Sequence")
    
    log("  Step 1: Panorama UI Open")
    send_masked(ws, PANORAMA_COMMANDS[0])
    time.sleep(1.0)
    
    log("  Step 2: Set ROW=3")
    send_masked(ws, PANORAMA_COMMANDS[1])
    time.sleep(1.0)
    
    log("  Step 3: Set COL=4")
    send_masked(ws, PANORAMA_COMMANDS[2])
    time.sleep(1.0)
    
    log("  Step 4: Start Panorama")
    send_masked(ws, PANORAMA_COMMANDS[3])
    log("")
    
    # Monitor progress
    log("PHASE 3: Monitor Progress (120s timeout)")
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
                        log("PANORAMA COMPLETE")
                        log(f"Expected: 3 x 4 = 12 images")
                        log(f"Got:      {total} images")
                        log("")
                        if total == 12:
                            log("✓✓✓ SUCCESS - Complete session replay works! ✓✓✓")
                            log("")
                            log("WORKING PROTOCOL IDENTIFIED:")
                            log("1. Send init commands (6 commands)")
                            log("2. Send Panorama UI Open")
                            log("3. Send ROW setting")
                            log("4. Send COL setting")
                            log("5. Send Panorama START")
                        else:
                            log(f"✗ FAILURE - Got {total} instead of 12")
                        log("="*80)
                        break
        except Exception as e:
            log(f"Error: {e}")
            break
    
    ws.close()
    log("\nSession complete.")
    logfile.close()
    
except Exception as e:
    log(f"ERROR: {e}")
    import traceback
    traceback.print_exc(file=logfile)
    logfile.close()
