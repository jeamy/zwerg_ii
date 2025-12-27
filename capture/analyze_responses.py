#!/usr/bin/env python3
"""
Analyze Server->Client responses to understand command flow
"""
import json

with open('/media/data/programming/zwergII/capture/ctrl_20251226_113958_full.json', 'r') as f:
    data = json.load(f)

c2s = data['c2s_commands']
s2c = data['s2c_commands']

output = open('/media/data/programming/zwergII/capture/response_analysis.txt', 'w')

def log(msg):
    output.write(msg + '\n')
    output.flush()

log("="*80)
log("COMMAND/RESPONSE FLOW ANALYSIS")
log("="*80)
log("")
log(f"Total C->S commands: {len(c2s)}")
log(f"Total S->C responses: {len(s2c)}")
log("")

# Create timeline
timeline = []
for cmd in c2s:
    timeline.append(('C->S', cmd['pkt_idx'], cmd['payload_hex']))
for resp in s2c:
    timeline.append(('S->C', resp['pkt_idx'], resp['payload_hex']))

timeline.sort(key=lambda x: x[1])

log("TIMELINE (packet order):")
log("="*80)

c2s_count = 0
s2c_count = 0

for direction, pkt_idx, payload_hex in timeline:
    if direction == 'C->S':
        c2s_count += 1
        payload = bytes.fromhex(payload_hex)
        
        # Detect command type
        cmd_type = ""
        if len(payload) >= 26:
            if payload[15] == 0x9c:
                cmd_type = f" [ROW={payload[25]}]"
            elif payload[15] == 0x9d:
                cmd_type = f" [COL={payload[25]}]"
        
        log(f"\nPacket {pkt_idx:3d} | C->S #{c2s_count:2d} | {len(payload):3d} bytes{cmd_type}")
        log(f"  Hex: {payload_hex[:80]}...")
    else:
        s2c_count += 1
        payload = bytes.fromhex(payload_hex)
        log(f"Packet {pkt_idx:3d} | S->C #{s2c_count:2d} | {len(payload):3d} bytes")
        log(f"  Hex: {payload_hex[:80]}...")

log("")
log("="*80)
log("COMMAND -> RESPONSE PATTERN")
log("="*80)

# Try to match C2S commands with following S2C responses
for i, cmd in enumerate(c2s):
    cmd_pkt = cmd['pkt_idx']
    
    # Find S2C responses that come after this command but before next command
    next_cmd_pkt = c2s[i+1]['pkt_idx'] if i+1 < len(c2s) else 999999
    
    responses = [r for r in s2c if cmd_pkt < r['pkt_idx'] < next_cmd_pkt]
    
    payload = bytes.fromhex(cmd['payload_hex'])
    cmd_desc = f"C2S #{i+1} (pkt {cmd_pkt}, {len(payload)} bytes)"
    
    if len(payload) >= 26:
        if payload[15] == 0x9c:
            cmd_desc += f" [ROW={payload[25]}]"
        elif payload[15] == 0x9d:
            cmd_desc += f" [COL={payload[25]}]"
    
    log(f"\n{cmd_desc}")
    
    if responses:
        log(f"  → {len(responses)} response(s):")
        for r in responses:
            log(f"     Packet {r['pkt_idx']}: {len(bytes.fromhex(r['payload_hex']))} bytes")
    else:
        log(f"  → NO immediate response")

log("\n✓ Done.")
output.close()
