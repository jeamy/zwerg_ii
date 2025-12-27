#!/usr/bin/env python3
"""Test panorama protobuf payload generation"""

import sys
sys.path.insert(0, '/media/data/programming/zwergII/capture')

import dwarf_proto_runtime as proto

# Test CommonParam für Rows
param_rows = proto.CommonParam(
    id=6,  # FEATURE_ID_PANO_ROW
    mode_index=1,
    continue_value=3.0
)

req_rows = proto.ReqSetFeatureParams(param=param_rows)
payload_rows = req_rows.SerializeToString()

print("=== PANORAMA ROWS (3) ===")
print(f"Payload length: {len(payload_rows)} bytes")
print(f"Hex: {payload_rows.hex()}")
print(f"Binary: {' '.join(f'{b:08b}' for b in payload_rows)}")
print()

# Test CommonParam für Cols
param_cols = proto.CommonParam(
    id=7,  # FEATURE_ID_PANO_COL
    mode_index=1,
    continue_value=4.0
)

req_cols = proto.ReqSetFeatureParams(param=param_cols)
payload_cols = req_cols.SerializeToString()

print("=== PANORAMA COLS (4) ===")
print(f"Payload length: {len(payload_cols)} bytes")
print(f"Hex: {payload_cols.hex()}")
print(f"Binary: {' '.join(f'{b:08b}' for b in payload_cols)}")
print()

# Manually decode protobuf structure
def decode_varint(data, offset):
    """Decode protobuf varint"""
    result = 0
    shift = 0
    while offset < len(data):
        byte = data[offset]
        result |= (byte & 0x7f) << shift
        offset += 1
        if (byte & 0x80) == 0:
            break
        shift += 7
    return result, offset

def decode_protobuf(data):
    """Simple protobuf decoder"""
    offset = 0
    fields = []
    while offset < len(data):
        if offset >= len(data):
            break
        tag, offset = decode_varint(data, offset)
        field_num = tag >> 3
        wire_type = tag & 0x7
        
        if wire_type == 0:  # Varint
            value, offset = decode_varint(data, offset)
            fields.append((field_num, 'varint', value))
        elif wire_type == 1:  # 64-bit
            value = int.from_bytes(data[offset:offset+8], 'little')
            offset += 8
            fields.append((field_num, '64-bit', value))
        elif wire_type == 2:  # Length-delimited
            length, offset = decode_varint(data, offset)
            value = data[offset:offset+length]
            offset += length
            fields.append((field_num, 'bytes', value))
        elif wire_type == 5:  # 32-bit
            value = int.from_bytes(data[offset:offset+4], 'little')
            offset += 4
            fields.append((field_num, '32-bit', value))
        else:
            print(f"Unknown wire type: {wire_type}")
            break
    return fields

print("=== DECODED ROWS PAYLOAD ===")
fields = decode_protobuf(payload_rows)
for field_num, wire_type, value in fields:
    if wire_type == 'bytes':
        print(f"Field {field_num} ({wire_type}): {value.hex()} (nested message)")
        nested = decode_protobuf(value)
        for nf, nw, nv in nested:
            if nw == '64-bit':
                import struct
                fval = struct.unpack('d', nv.to_bytes(8, 'little'))[0]
                print(f"  Field {nf} ({nw}): {nv} = {fval} (double)")
            else:
                print(f"  Field {nf} ({nw}): {nv}")
    else:
        print(f"Field {field_num} ({wire_type}): {value}")
print()

print("=== DECODED COLS PAYLOAD ===")
fields = decode_protobuf(payload_cols)
for field_num, wire_type, value in fields:
    if wire_type == 'bytes':
        print(f"Field {field_num} ({wire_type}): {value.hex()} (nested message)")
        nested = decode_protobuf(value)
        for nf, nw, nv in nested:
            if nw == '64-bit':
                import struct
                fval = struct.unpack('d', nv.to_bytes(8, 'little'))[0]
                print(f"  Field {nf} ({nw}): {nv} = {fval} (double)")
            else:
                print(f"  Field {nf} ({nw}): {nv}")
    else:
        print(f"Field {field_num} ({wire_type}): {value}")
