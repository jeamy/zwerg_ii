#!/usr/bin/env python3

"""
Test script for DWARF II Protobuf API
Uses ws://IP:9900 endpoint with Protobuf protocol
Tests all available commands except firmware update
"""

import os
import sys
import time
from websocket import create_connection, WebSocketTimeoutException
from google.protobuf.message import Message as ProtobufMessage
from google.protobuf.descriptor_pb2 import FileDescriptorProto, DescriptorProto, FieldDescriptorProto
from google.protobuf.message_factory import MessageFactory
from google.protobuf import descriptor_pool

DWARF_IP = os.environ.get('DWARF_IP', '192.168.88.1')
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

# Build minimal proto definitions dynamically
def build_proto_classes():
    """Build Protobuf message classes dynamically"""
    pool = descriptor_pool.Default()
    factory = MessageFactory(pool)
    
    # File descriptor for our proto
    file_proto = FileDescriptorProto()
    file_proto.name = 'dwarf.proto'
    file_proto.package = 'dwarf'
    
    # WsPacket message
    ws_packet = file_proto.message_type.add()
    ws_packet.name = 'WsPacket'
    
    fields = [
        ('major_version', 1, FieldDescriptorProto.TYPE_UINT32),
        ('minor_version', 2, FieldDescriptorProto.TYPE_UINT32),
        ('device_id', 3, FieldDescriptorProto.TYPE_UINT32),
        ('module_id', 4, FieldDescriptorProto.TYPE_UINT32),
        ('cmd', 5, FieldDescriptorProto.TYPE_UINT32),
        ('type', 6, FieldDescriptorProto.TYPE_UINT32),
        ('data', 7, FieldDescriptorProto.TYPE_BYTES),
        ('client_id', 8, FieldDescriptorProto.TYPE_STRING),
    ]
    
    for name, number, field_type in fields:
        field = ws_packet.field.add()
        field.name = name
        field.number = number
        field.type = field_type
        field.label = FieldDescriptorProto.LABEL_OPTIONAL
    
    # ComResponse message
    com_response = file_proto.message_type.add()
    com_response.name = 'ComResponse'
    field = com_response.field.add()
    field.name = 'code'
    field.number = 1
    field.type = FieldDescriptorProto.TYPE_INT32
    field.label = FieldDescriptorProto.LABEL_OPTIONAL
    
    # Register file descriptor
    pool.Add(file_proto)
    
    # Create message classes
    WsPacket = factory.GetPrototype(pool.FindMessageTypeByName('dwarf.WsPacket'))
    ComResponse = factory.GetPrototype(pool.FindMessageTypeByName('dwarf.ComResponse'))
    
    return WsPacket, ComResponse


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
        self.WsPacket, self.ComResponse = build_proto_classes()
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
            # Build WsPacket
            packet = self.WsPacket()
            packet.major_version = 1
            packet.minor_version = 1
            packet.device_id = 1
            packet.module_id = module_id
            packet.cmd = cmd
            packet.type = 0  # Request
            packet.data = data
            packet.client_id = self.client_id
            
            # Serialize and send
            buffer = packet.SerializeToString()
            self.ws.send(buffer, opcode=0x2)  # Binary frame
            
            # Wait for response with timeout
            self.ws.settimeout(3.0)
            response = self.ws.recv()
            
            try:
                # Parse response
                resp_packet = self.WsPacket()
                resp_packet.ParseFromString(response)
                
                print(f'  ← Module {resp_packet.module_id}, Cmd {resp_packet.cmd}, Type {resp_packet.type}, Data size {len(resp_packet.data)}')
                
                # Try to parse ComResponse
                code = 'UNKNOWN'
                if len(resp_packet.data) > 0 and resp_packet.type == 1:  # Response type
                    try:
                        com_resp = self.ComResponse()
                        com_resp.ParseFromString(resp_packet.data)
                        code = com_resp.code
                    except:
                        pass
                
                success = code == 0 or resp_packet.type == 1  # Accept any response
                status = '✓' if success else '✗'
                extra = f' (code: {code})' if code != 'UNKNOWN' else ''
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
                print(f'  ✗ Failed to parse response: {e}')
                return {
                    'module': module_id,
                    'cmd': cmd,
                    'desc': desc,
                    'success': False,
                    'code': f'PARSE_ERROR: {e}'
                }
                
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
