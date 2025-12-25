#!/usr/bin/env python3

"""
Test script for DWARF II Protobuf API
Uses ws://IP:9900 endpoint with manual Protobuf encoding
Tests all available commands except firmware update
"""

import os
import sys
import time
import struct
from websocket import create_connection, WebSocketTimeoutException

DWARF_IP = os.environ.get('DWARF_IP', '10.42.0.209')
WS_URL = f'ws://{DWARF_IP}:9900'

# Module IDs
MODULE_CAMERA_TELE = 1
MODULE_CAMERA_WIDE = 2
MODULE_ASTRO = 3
MODULE_SYSTEM = 4
MODULE_RGB_POWER = 5
MODULE_MOTOR = 6
MODULE_TRACK = 7
MODULE_FOCUS = 8
MODULE_NOTIFY = 9
MODULE_PANORAMA = 10


def encode_varint(value):
    """Encode integer as Protobuf varint"""
    result = bytearray()
    while value > 0x7f:
        result.append((value & 0x7f) | 0x80)
        value >>= 7
    result.append(value & 0x7f)
    return bytes(result)


def decode_varint(data, offset=0):
    """Decode Protobuf varint from data"""
    result = 0
    shift = 0
    pos = offset
    while True:
        if pos >= len(data):
            return None, pos
        byte = data[pos]
        pos += 1
        result |= (byte & 0x7f) << shift
        if not (byte & 0x80):
            break
        shift += 7
    return result, pos


def encode_ws_packet(major_version, minor_version, device_id, module_id, cmd, msg_type, data, client_id):
    """Manually encode WsPacket as Protobuf"""
    result = bytearray()
    
    # Field 1: major_version (uint32)
    if major_version:
        result.extend(b'\x08')  # tag: (1 << 3) | 0
        result.extend(encode_varint(major_version))
    
    # Field 2: minor_version (uint32)
    if minor_version:
        result.extend(b'\x10')  # tag: (2 << 3) | 0
        result.extend(encode_varint(minor_version))
    
    # Field 3: device_id (uint32)
    if device_id:
        result.extend(b'\x18')  # tag: (3 << 3) | 0
        result.extend(encode_varint(device_id))
    
    # Field 4: module_id (uint32)
    result.extend(b'\x20')  # tag: (4 << 3) | 0
    result.extend(encode_varint(module_id))
    
    # Field 5: cmd (uint32)
    result.extend(b'\x28')  # tag: (5 << 3) | 0
    result.extend(encode_varint(cmd))
    
    # Field 6: type (uint32)
    result.extend(b'\x30')  # tag: (6 << 3) | 0
    result.extend(encode_varint(msg_type))
    
    # Field 7: data (bytes) - always include even if empty
    result.extend(b'\x3a')  # tag: (7 << 3) | 2
    result.extend(encode_varint(len(data)))
    if data:
        result.extend(data)
    
    # Field 8: client_id (string)
    if client_id:
        client_bytes = client_id.encode('utf-8')
        result.extend(b'\x42')  # tag: (8 << 3) | 2
        result.extend(encode_varint(len(client_bytes)))
        result.extend(client_bytes)
    
    return bytes(result)


def decode_ws_packet(data):
    """Decode WsPacket from Protobuf data"""
    result = {}
    pos = 0
    
    while pos < len(data):
        # Read field tag
        tag, pos = decode_varint(data, pos)
        if tag is None:
            break
        
        field_num = tag >> 3
        wire_type = tag & 0x7
        
        if wire_type == 0:  # Varint
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
        
        elif wire_type == 2:  # Length-delimited
            length, pos = decode_varint(data, pos)
            if length is None:
                break
            value = data[pos:pos+length]
            pos += length
            
            if field_num == 7:
                result['data'] = value
            elif field_num == 8:
                result['client_id'] = value.decode('utf-8', errors='ignore')
        
        else:
            # Unknown wire type, skip
            break
    
    return result


def decode_com_response(data):
    """Decode ComResponse message (field 1 = code)"""
    if len(data) < 2:
        return None
    
    # Expect tag 0x08 (field 1, varint)
    if data[0] != 0x08:
        return None
    
    code, _ = decode_varint(data, 1)
    return code


# Command definitions (module_id, cmd, description)
COMMANDS = [
    # Telephoto Camera
    (MODULE_CAMERA_TELE, 10000, 'Open telephoto camera'),
    (MODULE_CAMERA_TELE, 10036, 'Get all tele params'),
    (MODULE_CAMERA_TELE, 10039, 'Get system working state'),
    (MODULE_CAMERA_TELE, 10008, 'Get exposure mode'),
    (MODULE_CAMERA_TELE, 10012, 'Get gain mode'),
    (MODULE_CAMERA_TELE, 10016, 'Get brightness'),
    (MODULE_CAMERA_TELE, 10018, 'Get contrast'),
    (MODULE_CAMERA_TELE, 10020, 'Get saturation'),
    (MODULE_CAMERA_TELE, 10022, 'Get hue'),
    (MODULE_CAMERA_TELE, 10024, 'Get sharpness'),
    (MODULE_CAMERA_TELE, 10026, 'Get white balance mode'),
    (MODULE_CAMERA_TELE, 10032, 'Get IRCUT status'),
    (MODULE_CAMERA_TELE, 10002, 'Take photo (tele)'),
    (MODULE_CAMERA_TELE, 10001, 'Close telephoto camera'),
    
    # Wide Camera
    (MODULE_CAMERA_WIDE, 12000, 'Open wide camera'),
    (MODULE_CAMERA_WIDE, 12027, 'Get all wide params'),
    (MODULE_CAMERA_WIDE, 12003, 'Get exposure mode (wide)'),
    (MODULE_CAMERA_WIDE, 12009, 'Get brightness (wide)'),
    (MODULE_CAMERA_WIDE, 12011, 'Get contrast (wide)'),
    (MODULE_CAMERA_WIDE, 12013, 'Get saturation (wide)'),
    (MODULE_CAMERA_WIDE, 12015, 'Get hue (wide)'),
    (MODULE_CAMERA_WIDE, 12017, 'Get sharpness (wide)'),
    (MODULE_CAMERA_WIDE, 12019, 'Get white balance mode (wide)'),
    (MODULE_CAMERA_WIDE, 12022, 'Take photo (wide)'),
    (MODULE_CAMERA_WIDE, 12001, 'Close wide camera'),
    
    # Focus
    (MODULE_FOCUS, 15000, 'Auto focus'),
    
    # Astronomy
    (MODULE_ASTRO, 11009, 'Check if dark frames exist'),
    (MODULE_ASTRO, 11023, 'Get dark frame list (tele)'),
    (MODULE_ASTRO, 11027, 'Get dark frame list (wide)'),
    
    # RGB & Power
    (MODULE_RGB_POWER, 13500, 'Open RGB light'),
    (MODULE_RGB_POWER, 13501, 'Close RGB light'),
]


class DwarfProtobufTester:
    def __init__(self):
        self.ws = None
        self.results = []
        self.client_id = f'test-{int(time.time())}'
        
    def connect(self):
        """Connect to DWARF II WebSocket"""
        print(f'Connecting to {WS_URL}...')
        try:
            self.ws = create_connection(WS_URL, timeout=5)
            print('✓ Connected (Protobuf mode)\n')
            return True
        except Exception as e:
            print(f'✗ Connection error: {e}')
            return False
    
    def send_command(self, module_id, cmd, desc, data=b''):
        """Send a Protobuf command and wait for response"""
        if not self.ws:
            return {'module': module_id, 'cmd': cmd, 'desc': desc, 'success': False, 'code': 'NO_CONNECTION'}
        
        print(f'→ {desc}')
        
        try:
            # Build and send packet
            packet = encode_ws_packet(
                major_version=1,
                minor_version=1,
                device_id=1,
                module_id=module_id,
                cmd=cmd,
                msg_type=0,  # Request
                data=data,
                client_id=self.client_id
            )
            
            self.ws.send(packet, opcode=0x2)  # Binary frame
            
            # Wait for response, ignoring notifications
            # Type 0 = Request, Type 1 = Response, Type 2 = Notification, Type 3 = Response
            self.ws.settimeout(3.0)
            max_attempts = 10  # Try up to 10 messages to find response
            
            for attempt in range(max_attempts):
                try:
                    response = self.ws.recv()
                except WebSocketTimeoutException:
                    break
                
                try:
                    # Parse response
                    resp_packet = decode_ws_packet(response)
                    
                    resp_module = resp_packet.get('module_id', 0)
                    resp_cmd = resp_packet.get('cmd', 0)
                    resp_type = resp_packet.get('type', 0)
                    resp_data = resp_packet.get('data', b'')
                    
                    # Skip notifications (Type 2, usually Module 9)
                    if resp_type == 2:
                        continue
                    
                    # This is a response (Type 1 or 3)
                    print(f'  ← Module {resp_module}, Cmd {resp_cmd}, Type {resp_type}, Data size {len(resp_data)}')
                    
                    # Try to parse ComResponse
                    code = 'UNKNOWN'
                    if len(resp_data) > 0 and resp_type in (1, 3):
                        code = decode_com_response(resp_data)
                        if code is None:
                            code = 'UNKNOWN'
                    
                    # Success if we got a response matching our request and code is 0
                    # Or if response has no data (empty response = success for some commands)
                    matches = (resp_module == module_id and resp_cmd == cmd)
                    success = matches and (code == 0 or len(resp_data) == 0)
                    
                    status = '✓' if success else '✗'
                    extra = f' (code: {code})' if code != 'UNKNOWN' else ''
                    if not matches:
                        extra += f' [mismatch: expected M{module_id}/C{cmd}]'
                    print(f'  {status} {desc}{extra}')
                    
                    return {
                        'module': module_id,
                        'cmd': cmd,
                        'desc': desc,
                        'success': success,
                        'code': code,
                        'response': resp_packet
                    }
                    
                except Exception as e:
                    print(f'  ✗ Failed to parse message: {e}')
                    continue
                
        except WebSocketTimeoutException:
            print(f'  ⚠ Timeout')
            return {
                'module': module_id,
                'cmd': cmd,
                'desc': desc,
                'success': False,
                'code': 'TIMEOUT'
            }
            
        except Exception as e:
            print(f'  ✗ Error: {e}')
            return {
                'module': module_id,
                'cmd': cmd,
                'desc': desc,
                'success': False,
                'code': str(e)
            }
    
    def run_tests(self):
        """Run all test commands"""
        print('═' * 60)
        print('  DWARF II Protobuf API Test')
        print('  (Complete command set via Protobuf)')
        print('═' * 60)
        print()
        
        categories = {
            'Telephoto Camera': MODULE_CAMERA_TELE,
            'Wide Camera': MODULE_CAMERA_WIDE,
            'Focus': MODULE_FOCUS,
            'Astronomy': MODULE_ASTRO,
            'RGB & Power': MODULE_RGB_POWER,
        }
        
        for category_name, module in categories.items():
            category_cmds = [c for c in COMMANDS if c[0] == module]
            if category_cmds:
                print(f'\n━━━ {category_name} ━━━')
                for module_id, cmd, desc in category_cmds:
                    result = self.send_command(module_id, cmd, desc)
                    self.results.append(result)
                    time.sleep(0.5)
    
    def print_summary(self):
        """Print test summary"""
        print('\n' + '═' * 60)
        print('  Test Summary')
        print('═' * 60)
        print()
        
        total = len(self.results)
        passed = sum(1 for r in self.results if r['success'])
        failed = total - passed
        
        print(f'Total commands tested: {total}')
        print(f'✓ Passed: {passed}')
        print(f'✗ Failed: {failed}')
        print()
        
        if failed > 0:
            print('Failed commands:')
            for r in self.results:
                if not r['success']:
                    print(f'  • {r["desc"]} ({r["code"]})')
            print()
        
        success_rate = (passed / total * 100) if total > 0 else 0
        print(f'Success rate: {success_rate:.1f}%')
    
    def close(self):
        """Close WebSocket connection"""
        if self.ws:
            self.ws.close()
            print('\n✓ Connection closed')


def main():
    """Main execution"""
    tester = DwarfProtobufTester()
    
    try:
        if not tester.connect():
            sys.exit(1)
        
        tester.run_tests()
        tester.print_summary()
        
    except KeyboardInterrupt:
        print('\n\nTest interrupted by user')
        sys.exit(1)
    except Exception as e:
        print(f'\nTest failed: {e}')
        import traceback
        traceback.print_exc()
        sys.exit(1)
    finally:
        tester.close()


if __name__ == '__main__':
    main()
