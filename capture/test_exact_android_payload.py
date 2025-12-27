#!/usr/bin/env python3
"""
Teste mit EXAKTEN Payloads aus Android PCAP Analyse (dump.md)
64-byte payload, Offset 15 = selector (0x9c/0x9d), Offset 25 = value
"""
import sys
import time
from websocket import create_connection

host = "10.42.0.209"
port = 9900

log = open('/media/data/programming/zwergII/capture/test_exact_payload.log', 'w', buffering=1)

def logmsg(msg):
    log.write(msg + '\n')
    log.flush()
    print(msg, flush=True)

# Template payload aus dump.md Analyse - Row Command mit Wert 5
# Dies ist das komplette 64-byte WebSocket Binary Payload (nach unmask)
TEMPLATE_64 = bytes.fromhex(
    "080110141801200f28bf8201"
    "3208011009120418061801201c28013001390000000000001c40"
    "419c808080e05e48014a0c089b8080808aa78a0210022001"
    "610000000000001440"
)

def build_pano_command(is_row, value):
    """
    Baue Panorama Command basierend auf Template
    - is_row: True für Row, False für Col
    - value: numerischer Wert (z.B. 3, 4, 5)
    """
    payload = bytearray(TEMPLATE_64)
    
    # Offset 15: selector
    payload[15] = 0x9c if is_row else 0x9d
    
    # Offset 25: value
    payload[25] = value
    
    return bytes(payload)

try:
    logmsg("="*80)
    logmsg(f"EXACT ANDROID PAYLOAD TEST")
    logmsg(f"Host: {host}:{port}")
    logmsg(f"Target: 3 rows x 4 cols")
    logmsg("="*80)
    logmsg("")
    
    ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
    logmsg("Connected!")
    logmsg("")
    
    # Send Row=3
    logmsg("STEP 1: Send ROW=3 (using exact Android payload format)")
    row_payload = build_pano_command(is_row=True, value=3)
    logmsg(f"  Payload length: {len(row_payload)} bytes")
    logmsg(f"  Offset 15 (selector): 0x{row_payload[15]:02x} (expect 0x9c for row)")
    logmsg(f"  Offset 25 (value): 0x{row_payload[25]:02x} (expect 0x03)")
    logmsg(f"  Full hex: {row_payload.hex()}")
    
    # WebSocket Binary Frame mit Mask
    # Client MUSS masken
    import os
    mask = os.urandom(4)
    masked = bytearray(row_payload)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    # Build WebSocket frame: FIN+opcode(2), mask+len, mask_key, masked_payload
    frame = bytearray()
    frame.append(0x82)  # FIN + Binary
    frame.append(0x80 | len(row_payload))  # MASK + length
    frame.extend(mask)
    frame.extend(masked)
    
    ws.send(bytes(frame), opcode=0x2)
    logmsg(f"  Sent {len(frame)} bytes (incl. WS frame header)")
    time.sleep(1)
    logmsg("")
    
    # Send Col=4
    logmsg("STEP 2: Send COL=4 (using exact Android payload format)")
    col_payload = build_pano_command(is_row=False, value=4)
    logmsg(f"  Payload length: {len(col_payload)} bytes")
    logmsg(f"  Offset 15 (selector): 0x{col_payload[15]:02x} (expect 0x9d for col)")
    logmsg(f"  Offset 25 (value): 0x{col_payload[25]:02x} (expect 0x04)")
    logmsg(f"  Full hex: {col_payload.hex()}")
    
    mask = os.urandom(4)
    masked = bytearray(col_payload)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    frame = bytearray()
    frame.append(0x82)
    frame.append(0x80 | len(col_payload))
    frame.extend(mask)
    frame.extend(masked)
    
    ws.send(bytes(frame), opcode=0x2)
    logmsg(f"  Sent {len(frame)} bytes (incl. WS frame header)")
    time.sleep(1)
    logmsg("")
    
    # Now start panorama (using simple WsPacket)
    logmsg("STEP 3: Start Panorama")
    # Build start command: module=10, cmd=15500
    sys.path.insert(0, '/media/data/programming/zwergII/capture')
    import dwarf_proto_runtime as proto
    
    def varint_encode(n):
        result = bytearray()
        while n > 0x7F:
            result.append((n & 0x7F) | 0x80)
            n >>= 7
        result.append(n & 0x7F)
        return bytes(result)
    
    pkt = proto.WsPacket(module_id=10, cmd=15500, type=0, data=b"")
    pkt_bytes = pkt.SerializeToString()
    msg = varint_encode(len(pkt_bytes)) + pkt_bytes
    
    mask = os.urandom(4)
    masked = bytearray(msg)
    for i in range(len(masked)):
        masked[i] ^= mask[i % 4]
    
    frame = bytearray()
    frame.append(0x82)
    frame.append(0x80 | len(msg))
    frame.extend(mask)
    frame.extend(masked)
    
    ws.send(bytes(frame), opcode=0x2)
    logmsg(f"  Sent {len(frame)} bytes")
    logmsg("")
    
    # Monitor progress
    logmsg("STEP 4: Monitor panorama progress (60s)")
    ws.settimeout(60.0)
    start_time = time.time()
    
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
    
    while time.time() - start_time < 60:
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
                    logmsg(f"  Progress: {current}/{total}")
                    if current >= total:
                        logmsg("")
                        logmsg("="*80)
                        logmsg("PANORAMA COMPLETE")
                        logmsg(f"Requested: 3 x 4 = 12 images")
                        logmsg(f"Actual: {total} images")
                        logmsg("")
                        if total == 12:
                            logmsg("✓✓✓ SUCCESS - Android payload format works! ✓✓✓")
                        else:
                            logmsg(f"✗ FAILURE - Still got {total} instead of 12")
                            if total == 25:
                                logmsg("  (Default 5x5 grid used)")
                        logmsg("="*80)
                        break
        except Exception as e:
            logmsg(f"  Error/Timeout: {e}")
            break
    
    ws.close()
    logmsg("")
    logmsg("Test complete.")
    log.close()
    
except Exception as e:
    logmsg(f"ERROR: {e}")
    import traceback
    traceback.print_exc()
    log.close()
