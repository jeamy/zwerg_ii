#!/usr/bin/env python3
"""
Dekodiere 64-byte Panorama Command Payload aus PCAP-Analyse
Header: 080110141801200f28bf8201...
"""

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

def varint_encode(n):
    result = bytearray()
    while n > 0x7F:
        result.append((n & 0x7F) | 0x80)
        n >>= 7
    result.append(n & 0x7F)
    return bytes(result)

# Example payload from dump.md analysis (64 bytes, unmaskiert)
# Dies ist ein KOMPLETTER WebSocket Binary Payload (nach unmask)
# Header: 080110141801200f28bf8201

# Aus dump.md - Row Command mit Wert 5 (0x9c an Offset 15, 0x05 an Offset 25)
example_payload = "080110141801200f28bf82013208011009120418061801201c28013001390000000000001c40419c808080e05e48014a0c089b8080808aa78a0210022001610000000000001440"

payload_bytes = bytes.fromhex(example_payload)

print(f"Payload length: {len(payload_bytes)} bytes")
print(f"Payload hex: {payload_bytes.hex()}")
print()

# Dies ist ein WebSocket Binary Frame Payload
# Erster Byte müsste varint length prefix sein für WsPacket
print("=== Decoding as WsPacket with length prefix ===")
pkt_len, offset = varint_decode(payload_bytes)
print(f"WsPacket length: {pkt_len} bytes (from varint at offset 0)")
print(f"Remaining bytes: {len(payload_bytes) - offset}")
print()

# Parse WsPacket fields
packet_data = payload_bytes[offset:offset+pkt_len]
print(f"WsPacket data: {packet_data.hex()}")
print()

print("=== WsPacket Fields ===")
off = 0
fields = {}
while off < len(packet_data):
    if off >= len(packet_data):
        break
    tag, off = varint_decode(packet_data, off)
    field_num = tag >> 3
    wire_type = tag & 0x7
    
    if wire_type == 0:  # Varint
        value, off = varint_decode(packet_data, off)
        fields[field_num] = value
        print(f"Field {field_num} (varint): {value}")
    elif wire_type == 1:  # 64-bit
        if off + 8 <= len(packet_data):
            import struct
            value = struct.unpack('<d', packet_data[off:off+8])[0]
            fields[f"{field_num}_double"] = value
            print(f"Field {field_num} (double): {value}")
            off += 8
    elif wire_type == 2:  # Length-delimited
        length, off = varint_decode(packet_data, off)
        if off + length <= len(packet_data):
            data = packet_data[off:off+length]
            fields[f"{field_num}_bytes"] = data
            print(f"Field {field_num} (bytes, len={length}): {data.hex()}")
            
            # Try to decode nested message
            if field_num == 4:  # data field in WsPacket
                print(f"  → Decoding nested payload:")
                noff = 0
                while noff < len(data):
                    ntag, noff = varint_decode(data, noff)
                    nfield = ntag >> 3
                    nwire = ntag & 0x7
                    
                    if nwire == 0:
                        nval, noff = varint_decode(data, noff)
                        print(f"    Field {nfield} (varint): {nval} (0x{nval:x})")
                    elif nwire == 1:
                        if noff + 8 <= len(data):
                            nval = struct.unpack('<d', data[noff:noff+8])[0]
                            print(f"    Field {nfield} (double): {nval}")
                            noff += 8
                    elif nwire == 2:
                        nlen, noff = varint_decode(data, noff)
                        if noff + nlen <= len(data):
                            ndata = data[noff:noff+nlen]
                            print(f"    Field {nfield} (bytes, len={nlen}): {ndata.hex()}")
                            noff += nlen
            
            off += length
    else:
        print(f"Unknown wire type {wire_type} at field {field_num}")
        break

print()
print("=== WsPacket Summary ===")
if 1 in fields:
    print(f"module_id: {fields[1]}")
if 2 in fields:
    print(f"cmd: {fields[2]}")
if 3 in fields:
    print(f"type: {fields[3]}")
if "4_bytes" in fields:
    print(f"data: {len(fields['4_bytes'])} bytes")

print()
print("=== Check Offset 15 and 25 (0-indexed in full payload) ===")
print(f"Byte at offset 15: 0x{payload_bytes[15]:02x}")
print(f"Byte at offset 25: 0x{payload_bytes[25]:02x}")

# Das 64-byte payload ist: length prefix + WsPacket
# WsPacket = module_id, cmd, type, data
# data enthält das eigentliche Command-Payload
# Offset 15 und 25 beziehen sich auf das GESAMTE 64-byte Payload
