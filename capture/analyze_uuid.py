#!/usr/bin/env python3
"""
Analyze UUID structure in commands
Compare across multiple PCAPs to determine if static or dynamic
"""
import json

output = open('/media/data/programming/zwergII/capture/uuid_analysis.txt', 'w')

def log(msg):
    output.write(msg + '\n')
    output.flush()

log("="*80)
log("UUID ANALYSIS")
log("="*80)
log("")

# Load commands from different PCAPs
pcaps = {
    'ctrl_20251226_1.pcapng': '/media/data/programming/zwergII/capture/ctrl_20251226_1_commands.json',
    'ctrl_20251226_113958.pcapng': '/media/data/programming/zwergII/capture/ctrl_20251226_113958_full.json'
}

uuid_samples = {}

for pcap_name, json_file in pcaps.items():
    try:
        with open(json_file, 'r') as f:
            data = json.load(f)
        
        c2s = data['c2s_commands']
        
        if c2s:
            # Extract UUID from first command
            first_cmd_hex = c2s[0]['payload_hex']
            
            # UUID is at the end of payload
            # Example: ...422432303939643762392d323537612d343166632d613161622d376535316165326630303030
            # The "4224..." part is the start of the UUID field
            
            # Find "4224" marker and extract following bytes
            payload_bytes = bytes.fromhex(first_cmd_hex)
            
            # Search for protobuf field tag (0x42 = field 8, wire type 2 = length-delimited)
            # followed by length byte (0x24 = 36 bytes for UUID string)
            marker = bytes([0x42, 0x24])
            pos = payload_bytes.find(marker)
            
            if pos >= 0:
                # Extract 36 bytes after marker
                uuid_bytes = payload_bytes[pos+2:pos+2+36]
                uuid_hex = uuid_bytes.hex()
                uuid_ascii = uuid_bytes.decode('ascii', errors='replace')
                
                uuid_samples[pcap_name] = {
                    'hex': uuid_hex,
                    'ascii': uuid_ascii,
                    'bytes': uuid_bytes
                }
                
                log(f"PCAP: {pcap_name}")
                log(f"  UUID Hex: {uuid_hex}")
                log(f"  UUID ASCII: {uuid_ascii}")
                log("")
    except Exception as e:
        log(f"Error processing {pcap_name}: {e}")
        log("")

# Compare UUIDs
log("="*80)
log("UUID COMPARISON")
log("="*80)
log("")

uuid_list = list(uuid_samples.values())

if len(uuid_list) >= 2:
    uuid1 = uuid_list[0]
    uuid2 = uuid_list[1]
    
    if uuid1['hex'] == uuid2['hex']:
        log("✓ UUIDs are IDENTICAL across PCAPs")
        log("  → UUID is STATIC (hardcoded or device-specific)")
        log("")
        log(f"Static UUID: {uuid1['ascii']}")
    else:
        log("✗ UUIDs are DIFFERENT across PCAPs")
        log("  → UUID is DYNAMIC (session-specific or time-based)")
        log("")
        log(f"PCAP 1: {uuid1['ascii']}")
        log(f"PCAP 2: {uuid2['ascii']}")
        log("")
        
        # Analyze differences
        log("Difference Analysis:")
        for i, (b1, b2) in enumerate(zip(uuid1['bytes'], uuid2['bytes'])):
            if b1 != b2:
                log(f"  Position {i}: {chr(b1)} ({b1:02x}) -> {chr(b2)} ({b2:02x})")
else:
    log("Not enough samples for comparison")

log("")
log("="*80)
log("UUID STRUCTURE ANALYSIS")
log("="*80)
log("")

if uuid_list:
    uuid_ascii = uuid_list[0]['ascii']
    
    log(f"UUID String: {uuid_ascii}")
    log(f"Length: {len(uuid_ascii)} chars")
    log("")
    
    # Check if it looks like a standard UUID format
    if '-' in uuid_ascii:
        parts = uuid_ascii.split('-')
        log("UUID Format:")
        log(f"  Parts: {len(parts)}")
        for i, part in enumerate(parts):
            log(f"  Part {i+1}: {part} ({len(part)} chars)")
        
        # Standard UUID v4 format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
        if len(parts) == 5:
            log("")
            log("Matches UUID format (8-4-4-4-12)")
            
            # Check version
            if len(parts[2]) >= 1:
                version_char = parts[2][0]
                log(f"UUID Version indicator: {version_char}")
                if version_char == '4':
                    log("  → Appears to be UUID v4 (random)")
    else:
        log("Non-standard UUID format (no dashes)")

log("")
log("="*80)
log("RECOMMENDATIONS")
log("="*80)
log("")

if len(uuid_list) >= 2 and uuid_list[0]['hex'] == uuid_list[1]['hex']:
    log("UUID is STATIC - can be hardcoded in replay scripts")
    log("")
    log("Recommended action:")
    log("  1. Use this UUID in all commands:")
    log(f"     {uuid_list[0]['ascii']}")
    log("  2. Test if DWARF accepts commands with this UUID")
    log("  3. If accepted: Problem is elsewhere (not UUID-related)")
    log("  4. If rejected: Try extracting UUID from DWARF at connection time")
else:
    log("UUID is DYNAMIC - must be generated or retrieved")
    log("")
    log("Recommended actions:")
    log("  1. Check if DWARF sends UUID during connection handshake")
    log("  2. Analyze S->C responses for UUID patterns")
    log("  3. If no UUID in responses: Generate using same algorithm as Android")
    log("  4. If UUID v4: Generate random UUID v4 for each session")

log("")
log("✓ Done.")
output.close()
