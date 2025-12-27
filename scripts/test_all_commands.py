#!/usr/bin/env python3

"""
Test script for all DWARF II commands (except firmware update)
Systematically tests every available command to verify protocol implementation
"""

import json
import os
import sys
import time
from datetime import datetime
from websocket import create_connection, WebSocketTimeoutException

DWARF_IP = os.environ.get('DWARF_IP', '10.42.0.209')
WS_URL = f'ws://{DWARF_IP}:9900/'

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
MODULE_SHOOTING_SCHEDULE = 13

# Command definitions
COMMANDS = {
    # Telephoto Camera (10000-10499)
    'TELE_OPEN_CAMERA': {'module': MODULE_CAMERA_TELE, 'cmd': 10000, 'desc': 'Open telephoto camera'},
    'TELE_GET_ALL_PARAMS': {'module': MODULE_CAMERA_TELE, 'cmd': 10036, 'desc': 'Get all tele params'},
    'TELE_GET_SYSTEM_STATE': {'module': MODULE_CAMERA_TELE, 'cmd': 10039, 'desc': 'Get system working state'},
    'TELE_GET_EXP_MODE': {'module': MODULE_CAMERA_TELE, 'cmd': 10008, 'desc': 'Get exposure mode'},
    'TELE_GET_GAIN_MODE': {'module': MODULE_CAMERA_TELE, 'cmd': 10012, 'desc': 'Get gain mode'},
    'TELE_GET_BRIGHTNESS': {'module': MODULE_CAMERA_TELE, 'cmd': 10016, 'desc': 'Get brightness'},
    'TELE_GET_CONTRAST': {'module': MODULE_CAMERA_TELE, 'cmd': 10018, 'desc': 'Get contrast'},
    'TELE_GET_SATURATION': {'module': MODULE_CAMERA_TELE, 'cmd': 10020, 'desc': 'Get saturation'},
    'TELE_GET_HUE': {'module': MODULE_CAMERA_TELE, 'cmd': 10022, 'desc': 'Get hue'},
    'TELE_GET_SHARPNESS': {'module': MODULE_CAMERA_TELE, 'cmd': 10024, 'desc': 'Get sharpness'},
    'TELE_GET_WB_MODE': {'module': MODULE_CAMERA_TELE, 'cmd': 10026, 'desc': 'Get white balance mode'},
    'TELE_GET_IRCUT': {'module': MODULE_CAMERA_TELE, 'cmd': 10032, 'desc': 'Get IRCUT status'},
    'TELE_PHOTOGRAPH': {'module': MODULE_CAMERA_TELE, 'cmd': 10002, 'desc': 'Take photo (tele)'},
    'TELE_CLOSE_CAMERA': {'module': MODULE_CAMERA_TELE, 'cmd': 10001, 'desc': 'Close telephoto camera'},
    
    # Wide Camera (12000-12499)
    'WIDE_OPEN_CAMERA': {'module': MODULE_CAMERA_WIDE, 'cmd': 12000, 'desc': 'Open wide camera'},
    'WIDE_GET_ALL_PARAMS': {'module': MODULE_CAMERA_WIDE, 'cmd': 12027, 'desc': 'Get all wide params'},
    'WIDE_GET_EXP_MODE': {'module': MODULE_CAMERA_WIDE, 'cmd': 12003, 'desc': 'Get exposure mode (wide)'},
    'WIDE_GET_BRIGHTNESS': {'module': MODULE_CAMERA_WIDE, 'cmd': 12009, 'desc': 'Get brightness (wide)'},
    'WIDE_GET_CONTRAST': {'module': MODULE_CAMERA_WIDE, 'cmd': 12011, 'desc': 'Get contrast (wide)'},
    'WIDE_GET_SATURATION': {'module': MODULE_CAMERA_WIDE, 'cmd': 12013, 'desc': 'Get saturation (wide)'},
    'WIDE_GET_HUE': {'module': MODULE_CAMERA_WIDE, 'cmd': 12015, 'desc': 'Get hue (wide)'},
    'WIDE_GET_SHARPNESS': {'module': MODULE_CAMERA_WIDE, 'cmd': 12017, 'desc': 'Get sharpness (wide)'},
    'WIDE_GET_WB_MODE': {'module': MODULE_CAMERA_WIDE, 'cmd': 12019, 'desc': 'Get white balance mode (wide)'},
    'WIDE_PHOTOGRAPH': {'module': MODULE_CAMERA_WIDE, 'cmd': 12022, 'desc': 'Take photo (wide)'},
    'WIDE_CLOSE_CAMERA': {'module': MODULE_CAMERA_WIDE, 'cmd': 12001, 'desc': 'Close wide camera'},
    
    # Focus (15000-15099)
    'FOCUS_AUTO': {'module': MODULE_FOCUS, 'cmd': 15000, 'desc': 'Auto focus'},
    
    # Astronomy - Read-only commands (11000-11499)
    'ASTRO_CHECK_DARK': {'module': MODULE_ASTRO, 'cmd': 11009, 'desc': 'Check if dark frames exist'},
    'ASTRO_GET_DARK_LIST': {'module': MODULE_ASTRO, 'cmd': 11023, 'desc': 'Get dark frame list (tele)'},
    'ASTRO_GET_WIDE_DARK_LIST': {'module': MODULE_ASTRO, 'cmd': 11027, 'desc': 'Get dark frame list (wide)'},
    
    # System (13000-13299)
    'SYSTEM_SET_TIME': {'module': MODULE_SYSTEM, 'cmd': 13000, 'desc': 'Set system time', 'payload': 'get_system_time_payload'},
    'SYSTEM_SET_TIMEZONE': {'module': MODULE_SYSTEM, 'cmd': 13001, 'desc': 'Set timezone', 'payload': {'timezone': 3600}},
    
    # RGB & Power (13500-13799)
    'RGB_OPEN': {'module': MODULE_RGB_POWER, 'cmd': 13500, 'desc': 'Open RGB light'},
    'RGB_CLOSE': {'module': MODULE_RGB_POWER, 'cmd': 13501, 'desc': 'Close RGB light'},
    
    # Panorama (15500-15599)
    'PANORAMA_START_GRID': {'module': MODULE_PANORAMA, 'cmd': 15500, 'desc': 'Start panorama grid 3x3', 'payload': {'mode': 0, 'rows': 3, 'cols': 3, 'overlap': 0.25}},
    'PANORAMA_STOP': {'module': MODULE_PANORAMA, 'cmd': 15501, 'desc': 'Stop panorama'}
}


def get_system_time_payload():
    """Generate current system time payload"""
    now = datetime.now()
    return {
        'year': now.year,
        'month': now.month,
        'day': now.day,
        'hour': now.hour,
        'min': now.minute,
        'sec': now.second
    }


class DwarfTester:
    def __init__(self):
        self.ws = None
        self.results = []
        
    def connect(self):
        """Connect to DWARF II WebSocket"""
        print(f'Connecting to {WS_URL}...')
        try:
            self.ws = create_connection(WS_URL, timeout=5)
            print('✓ Connected\n')
            return True
        except Exception as e:
            print(f'✗ Connection error: {e}')
            return False
    
    def send_command(self, name, config):
        """Send a command and wait for response"""
        if not self.ws:
            return {'command': name, 'desc': config['desc'], 'success': False, 'code': 'NO_CONNECTION'}
        
        # Build payload
        payload = {}
        if 'payload' in config:
            if isinstance(config['payload'], dict):
                payload = config['payload']
            elif config['payload'] == 'get_system_time_payload':
                payload = get_system_time_payload()
        
        # Build packet
        packet = {
            'cmd': config['cmd'],
            'module': config['module'],
            **payload
        }
        
        print(f'→ {config["desc"]}')
        
        try:
            # Send command
            self.ws.send(json.dumps(packet))
            
            # Wait for response with timeout
            self.ws.settimeout(3.0)
            response = self.ws.recv()
            
            try:
                msg = json.loads(response)
                success = msg.get('code', -1) == 0
                result = {
                    'command': name,
                    'desc': config['desc'],
                    'success': success,
                    'code': msg.get('code', 'UNKNOWN'),
                    'response': msg
                }
                
                status = '✓' if success else '✗'
                extra = '' if success else f' (code: {result["code"]})'
                print(f'  {status} {result["desc"]}{extra}')
                
                return result
                
            except json.JSONDecodeError:
                # Binary or non-JSON response
                result = {
                    'command': name,
                    'desc': config['desc'],
                    'success': False,
                    'code': 'INVALID_JSON',
                    'response': None
                }
                print(f'  ✗ Invalid JSON response')
                return result
                
        except WebSocketTimeoutException:
            result = {
                'command': name,
                'desc': config['desc'],
                'success': False,
                'code': 'TIMEOUT',
                'response': None
            }
            print(f'  ⚠ Timeout')
            return result
            
        except Exception as e:
            result = {
                'command': name,
                'desc': config['desc'],
                'success': False,
                'code': str(e),
                'response': None
            }
            print(f'  ✗ Error: {e}')
            return result
    
    def run_tests(self):
        """Run all test commands"""
        print('═' * 55)
        print('  DWARF II Complete Command Test Suite')
        print('  (Excluding firmware update)')
        print('═' * 55)
        print()
        
        categories = [
            {
                'name': 'Telephoto Camera',
                'commands': [
                    'TELE_OPEN_CAMERA',
                    'TELE_GET_ALL_PARAMS',
                    'TELE_GET_SYSTEM_STATE',
                    'TELE_GET_EXP_MODE',
                    'TELE_GET_GAIN_MODE',
                    'TELE_GET_BRIGHTNESS',
                    'TELE_GET_CONTRAST',
                    'TELE_GET_SATURATION',
                    'TELE_GET_HUE',
                    'TELE_GET_SHARPNESS',
                    'TELE_GET_WB_MODE',
                    'TELE_GET_IRCUT',
                    'TELE_PHOTOGRAPH',
                    'TELE_CLOSE_CAMERA'
                ]
            },
            {
                'name': 'Wide Camera',
                'commands': [
                    'WIDE_OPEN_CAMERA',
                    'WIDE_GET_ALL_PARAMS',
                    'WIDE_GET_EXP_MODE',
                    'WIDE_GET_BRIGHTNESS',
                    'WIDE_GET_CONTRAST',
                    'WIDE_GET_SATURATION',
                    'WIDE_GET_HUE',
                    'WIDE_GET_SHARPNESS',
                    'WIDE_GET_WB_MODE',
                    'WIDE_PHOTOGRAPH',
                    'WIDE_CLOSE_CAMERA'
                ]
            },
            {
                'name': 'Focus',
                'commands': ['FOCUS_AUTO']
            },
            {
                'name': 'Astronomy',
                'commands': [
                    'ASTRO_CHECK_DARK',
                    'ASTRO_GET_DARK_LIST',
                    'ASTRO_GET_WIDE_DARK_LIST'
                ]
            },
            {
                'name': 'System',
                'commands': [
                    'SYSTEM_SET_TIME',
                    'SYSTEM_SET_TIMEZONE'
                ]
            },
            {
                'name': 'RGB & Power',
                'commands': [
                    'RGB_OPEN',
                    'RGB_CLOSE'
                ]
            },
            {
                'name': 'Panorama',
                'commands': [
                    'PANORAMA_START_GRID',
                    'PANORAMA_STOP'
                ]
            }
        ]
        
        for category in categories:
            print(f'\n━━━ {category["name"]} ━━━')
            
            for cmd_name in category['commands']:
                config = COMMANDS.get(cmd_name)
                if not config:
                    print(f'  ⚠ Command not found: {cmd_name}')
                    continue
                
                result = self.send_command(cmd_name, config)
                self.results.append(result)
                time.sleep(0.5)  # Small delay between commands
    
    def print_summary(self):
        """Print test summary"""
        print('\n' + '═' * 55)
        print('  Test Summary')
        print('═' * 55)
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
    tester = DwarfTester()
    
    try:
        if not tester.connect():
            sys.exit(1)
        
        tester.run_tests()
        time.sleep(2)  # Wait for last responses
        tester.print_summary()
        
    except KeyboardInterrupt:
        print('\n\nTest interrupted by user')
        sys.exit(1)
    except Exception as e:
        print(f'\nTest failed: {e}')
        sys.exit(1)
    finally:
        tester.close()


if __name__ == '__main__':
    main()
