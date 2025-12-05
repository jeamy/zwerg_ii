#!/usr/bin/env python3
"""
Test IR-Cut toggle - this should have a visible effect on the camera stream.
IR-Cut OFF (value=1) = Night mode, image should look reddish
IR-Cut ON (value=0) = Day mode, normal colors
"""

import asyncio
import websockets
import uuid
import sys

DWARF_IP = "192.168.8.223"
DWARF_WS_PORT = 9900

MAJOR_VERSION = 1
MINOR_VERSION = 1
DEVICE_ID = 1

MODULE_CAMERA_TELE = 1
CMD_TELE_SET_IRCUT = 10031
CMD_TELE_GET_IRCUT = 10032

CLIENT_ID = str(uuid.uuid4())

def encode_varint(value):
    result = bytearray()
    if value < 0:
        value = value & 0xFFFFFFFFFFFFFFFF
    while value >= 0x80:
        result.append((value & 0x7F) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)

def encode_string(field_num, s):
    tag = (field_num << 3) | 2
    data = s.encode('utf-8')
    return encode_varint(tag) + encode_varint(len(data)) + data

def encode_bytes(field_num, data):
    tag = (field_num << 3) | 2
    return encode_varint(tag) + encode_varint(len(data)) + data

def encode_uint32(field_num, value):
    tag = (field_num << 3) | 0
    return encode_varint(tag) + encode_varint(value)

def create_ws_packet(module_id, cmd_id, data=b''):
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
    result = {'module_id': 0, 'cmd': 0, 'type': 0, 'data': b''}
    pos = 0
    while pos < len(data):
        tag, pos = decode_varint(data, pos)
        field_num = tag >> 3
        wire_type = tag & 0x7
        if wire_type == 0:
            value, pos = decode_varint(data, pos)
            if field_num == 4: result['module_id'] = value
            elif field_num == 5: result['cmd'] = value
            elif field_num == 6: result['type'] = value
        elif wire_type == 2:
            length, pos = decode_varint(data, pos)
            value = data[pos:pos+length]
            pos += length
            if field_num == 7: result['data'] = value
    return result

async def toggle_ircut(value):
    uri = f"ws://{DWARF_IP}:{DWARF_WS_PORT}"
    print(f"Connecting to {uri}...", flush=True)
    
    async with websockets.connect(uri) as ws:
        print("Connected!", flush=True)
        
        # Set IR-Cut
        # value=0: IR-Cut ON (day mode, blocks IR)
        # value=1: IR-Cut OFF (night mode, passes IR - image looks reddish)
        data = encode_uint32(1, value)  # field 1 = value
        packet = create_ws_packet(MODULE_CAMERA_TELE, CMD_TELE_SET_IRCUT, data)
        await ws.send(packet)
        print(f"Sent SET_IRCUT={value} (cmd={CMD_TELE_SET_IRCUT})", flush=True)
        
        # Wait for response
        try:
            while True:
                response = await asyncio.wait_for(ws.recv(), timeout=3.0)
                parsed = parse_ws_packet(response)
                print(f"Response: module={parsed['module_id']}, cmd={parsed['cmd']}, type={parsed['type']}", flush=True)
                if parsed['cmd'] == CMD_TELE_SET_IRCUT and parsed['type'] == 3:
                    print("IR-Cut command acknowledged!", flush=True)
                    break
        except asyncio.TimeoutError:
            print("Timeout waiting for response", flush=True)
        
        print(f"\nIR-Cut is now {'OFF (night mode)' if value == 1 else 'ON (day mode)'}", flush=True)
        print("Check the camera stream for visible changes!", flush=True)

if __name__ == "__main__":
    value = 1  # Default: turn IR-Cut OFF (night mode)
    if len(sys.argv) > 1:
        value = int(sys.argv[1])
    
    print(f"Setting IR-Cut to {value} ({'OFF/night' if value == 1 else 'ON/day'})", flush=True)
    asyncio.run(toggle_ircut(value))
