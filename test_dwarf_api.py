#!/usr/bin/env python3
"""
Test script to directly test DWARF II camera API commands.
This helps debug why camera settings are not being applied.
"""

import asyncio
import websockets
import uuid

DWARF_IP = "192.168.8.223"
DWARF_WS_PORT = 9900

# Protocol constants
MAJOR_VERSION = 1
MINOR_VERSION = 1
DEVICE_ID = 1  # DWARF II

# Module IDs
MODULE_CAMERA_TELE = 1
MODULE_CAMERA_WIDE = 2

# Command IDs for Tele camera (from official API documentation)
CMD_TELE_OPEN_CAMERA = 10000
CMD_TELE_CLOSE_CAMERA = 10001
CMD_TELE_PHOTOGRAPH = 10002
CMD_TELE_SET_EXP_MODE = 10007
CMD_TELE_GET_EXP_MODE = 10008
CMD_TELE_SET_EXP = 10009
CMD_TELE_GET_EXP = 10010
CMD_TELE_SET_GAIN_MODE = 10011
CMD_TELE_GET_GAIN_MODE = 10012
CMD_TELE_SET_GAIN = 10013
CMD_TELE_GET_GAIN = 10014
CMD_TELE_SET_BRIGHTNESS = 10015  # Correct!
CMD_TELE_GET_BRIGHTNESS = 10016
CMD_TELE_SET_CONTRAST = 10017
CMD_TELE_GET_CONTRAST = 10018
CMD_TELE_SET_SATURATION = 10019
CMD_TELE_GET_SATURATION = 10020
CMD_TELE_SET_HUE = 10021
CMD_TELE_GET_HUE = 10022
CMD_TELE_SET_SHARPNESS = 10023
CMD_TELE_GET_SHARPNESS = 10024
CMD_TELE_SET_WB_MODE = 10025
CMD_TELE_GET_WB_MODE = 10026
CMD_TELE_SET_WB_SCENE = 10027
CMD_TELE_GET_WB_SCENE = 10028
CMD_TELE_SET_WB_CT = 10029
CMD_TELE_GET_WB_CT = 10030
CMD_TELE_SET_IRCUT = 10031  # Correct!
CMD_TELE_GET_IRCUT = 10032
CMD_TELE_SET_ALL_PARAMS = 10035
CMD_TELE_GET_ALL_PARAMS = 10036

CLIENT_ID = str(uuid.uuid4())

def encode_varint(value):
    """Encode an integer as a protobuf varint."""
    result = bytearray()
    if value < 0:
        value = value & 0xFFFFFFFFFFFFFFFF  # Convert to unsigned
    while value >= 0x80:
        result.append((value & 0x7F) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)

def encode_string(field_num, s):
    """Encode a string field."""
    tag = (field_num << 3) | 2  # wire type 2 = length-delimited
    data = s.encode('utf-8')
    return encode_varint(tag) + encode_varint(len(data)) + data

def encode_bytes(field_num, data):
    """Encode a bytes field."""
    tag = (field_num << 3) | 2  # wire type 2 = length-delimited
    return encode_varint(tag) + encode_varint(len(data)) + data

def encode_uint32(field_num, value):
    """Encode a uint32 field."""
    tag = (field_num << 3) | 0  # wire type 0 = varint
    return encode_varint(tag) + encode_varint(value)

def create_protobuf_int32(field_num, value):
    """Create a protobuf message with a single int32 field."""
    return encode_uint32(field_num, value)

def create_ws_packet(module_id, cmd_id, data=b''):
    """Create a DWARF II WsPacket protobuf message."""
    # WsPacket fields:
    # 1: major_version (uint32)
    # 2: minor_version (uint32)
    # 3: device_id (uint32)
    # 4: module_id (uint32)
    # 5: cmd (uint32)
    # 6: type (uint32) - 0=request
    # 7: data (bytes)
    # 8: client_id (string)
    
    packet = b''
    packet += encode_uint32(1, MAJOR_VERSION)
    packet += encode_uint32(2, MINOR_VERSION)
    packet += encode_uint32(3, DEVICE_ID)
    packet += encode_uint32(4, module_id)
    packet += encode_uint32(5, cmd_id)
    packet += encode_uint32(6, 0)  # type = request
    if data:
        packet += encode_bytes(7, data)
    packet += encode_string(8, CLIENT_ID)
    
    return packet

def decode_varint(data, pos):
    """Decode a varint from data starting at pos."""
    result = 0
    shift = 0
    while pos < len(data):
        b = data[pos]
        result |= (b & 0x7F) << shift
        pos += 1
        if (b & 0x80) == 0:
            break
        shift += 7
    return result, pos

def parse_ws_packet(data):
    """Parse a WsPacket protobuf message."""
    result = {
        'major_version': 0,
        'minor_version': 0,
        'device_id': 0,
        'module_id': 0,
        'cmd': 0,
        'type': 0,
        'data': b'',
        'client_id': ''
    }
    
    pos = 0
    while pos < len(data):
        tag, pos = decode_varint(data, pos)
        field_num = tag >> 3
        wire_type = tag & 0x7
        
        if wire_type == 0:  # varint
            value, pos = decode_varint(data, pos)
            if field_num == 1:
                result['major_version'] = value
            elif field_num == 2:
                result['minor_version'] = value
            elif field_num == 3:
                result['device_id'] = value
            elif field_num == 4:
                result['module_id'] = value
            elif field_num == 5:
                result['cmd'] = value
            elif field_num == 6:
                result['type'] = value
        elif wire_type == 2:  # length-delimited
            length, pos = decode_varint(data, pos)
            value = data[pos:pos+length]
            pos += length
            if field_num == 7:
                result['data'] = value
            elif field_num == 8:
                result['client_id'] = value.decode('utf-8', errors='replace')
    
    return result

async def test_camera_commands():
    """Test various camera commands."""
    uri = f"ws://{DWARF_IP}:{DWARF_WS_PORT}"
    
    print(f"Connecting to {uri}...", flush=True)
    print(f"Using client_id: {CLIENT_ID}", flush=True)
    
    try:
        async with websockets.connect(uri) as ws:
            print("Connected!", flush=True)
            
            # Test 1: Get current exposure mode
            print("\n=== Test 1: GET_EXP_MODE ===", flush=True)
            packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_GET_EXP_MODE)
            await ws.send(packet)
            print(f"Sent GET_EXP_MODE (cmd={CMD_TELE_GET_EXP_MODE}), packet_hex={packet.hex()}", flush=True)
            
            try:
                response = await asyncio.wait_for(ws.recv(), timeout=2.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data={parsed['data'].hex()}", flush=True)
            except asyncio.TimeoutError:
                print("No response (timeout)", flush=True)
            
            # Test 2: Set exposure mode to Manual (1)
            print("\n=== Test 2: SET_EXP_MODE = 1 (Manual) ===", flush=True)
            data = create_protobuf_int32(1, 1)  # field 1 = mode, value = 1 (manual)
            packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_SET_EXP_MODE, data)
            await ws.send(packet)
            print(f"Sent SET_EXP_MODE=1 (cmd={CMD_TELE_SET_EXP_MODE}), inner_data={data.hex()}", flush=True)
            
            try:
                response = await asyncio.wait_for(ws.recv(), timeout=2.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data={parsed['data'].hex()}", flush=True)
            except asyncio.TimeoutError:
                print("No response (timeout)", flush=True)
            
            # Test 3: Set exposure index to 120 (1 second)
            print("\n=== Test 3: SET_EXP index=120 (1s) ===", flush=True)
            data = create_protobuf_int32(1, 120)  # field 1 = index, value = 120
            packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_SET_EXP, data)
            await ws.send(packet)
            print(f"Sent SET_EXP=120 (cmd={CMD_TELE_SET_EXP}), inner_data={data.hex()}", flush=True)
            
            try:
                response = await asyncio.wait_for(ws.recv(), timeout=2.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data={parsed['data'].hex()}", flush=True)
            except asyncio.TimeoutError:
                print("No response (timeout)", flush=True)
            
            # Test 4: Set brightness to 200 (high)
            print("\n=== Test 4: SET_BRIGHTNESS = 200 ===", flush=True)
            data = create_protobuf_int32(1, 200)  # field 1 = value
            packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_SET_BRIGHTNESS, data)
            await ws.send(packet)
            print(f"Sent SET_BRIGHTNESS=200 (cmd={CMD_TELE_SET_BRIGHTNESS}), inner_data={data.hex()}", flush=True)
            
            try:
                response = await asyncio.wait_for(ws.recv(), timeout=2.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data={parsed['data'].hex()}", flush=True)
            except asyncio.TimeoutError:
                print("No response (timeout)", flush=True)
            
            # Test 5: Get brightness
            print("\n=== Test 5: GET_BRIGHTNESS ===", flush=True)
            packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_GET_BRIGHTNESS)
            await ws.send(packet)
            print(f"Sent GET_BRIGHTNESS (cmd={CMD_TELE_GET_BRIGHTNESS})", flush=True)
            
            try:
                response = await asyncio.wait_for(ws.recv(), timeout=2.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data={parsed['data'].hex()}", flush=True)
            except asyncio.TimeoutError:
                print("No response (timeout)", flush=True)
            
            # Test 6: Set IR-Cut to PASS (1) - should make image reddish
            print("\n=== Test 6: SET_IRCUT = 1 (PASS) ===", flush=True)
            data = create_protobuf_int32(1, 1)  # field 1 = value, 1 = PASS
            packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_SET_IRCUT, data)
            await ws.send(packet)
            print(f"Sent SET_IRCUT=1 (cmd={CMD_TELE_SET_IRCUT}), inner_data={data.hex()}", flush=True)
            
            try:
                response = await asyncio.wait_for(ws.recv(), timeout=2.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data={parsed['data'].hex()}", flush=True)
            except asyncio.TimeoutError:
                print("No response (timeout)", flush=True)
            
            # Wait a bit and listen for any additional messages
            print("\n=== Listening for additional messages (5s) ===", flush=True)
            try:
                while True:
                    response = await asyncio.wait_for(ws.recv(), timeout=5.0)
                    parsed = parse_ws_packet(response)
                    print(f"Received: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}, data_len={len(parsed['data'])}", flush=True)
            except asyncio.TimeoutError:
                print("No more messages", flush=True)
            
    except Exception as e:
        print(f"Error: {e}", flush=True)
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    asyncio.run(test_camera_commands())
