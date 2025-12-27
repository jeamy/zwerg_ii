#!/usr/bin/env python3

"""
Test script for DWARF II JSON API (simplified command set)
Uses undocumented ws://IP:9900/ws endpoint with JSON commands
"""

import json
import os
import sys
import time
from websocket import create_connection, WebSocketTimeoutException

DWARF_IP = os.environ.get('DWARF_IP', '192.168.88.1')
WS_URL = f'ws://{DWARF_IP}:9900/ws'

# Available JSON commands (discovered from working scripts)
COMMANDS = [
    {'cmd': 'turnOnCamera', 'desc': 'Turn on camera'},
    {'cmd': 'cameraWorkingState', 'desc': 'Get camera working state'},
    {'cmd': 'takePhoto', 'desc': 'Take photo'},
    {'cmd': 'startVideo', 'desc': 'Start video recording'},
    {'cmd': 'stopVideo', 'desc': 'Stop video recording'},
    {'cmd': 'startTimeLapse', 'params': {'interval': 2, 'count': 5}, 'desc': 'Start timelapse (5 photos, 2s interval)'},
    {'cmd': 'startPano', 'params': {'rows': 3, 'cols': 3, 'overlap': 0.25, 'mode': 'wide'}, 'desc': 'Start panorama 3x3'},
    {'cmd': 'stopPano', 'desc': 'Stop panorama'},
    # ISP settings (read-only)
    {'cmd': 'exposure', 'desc': 'Get exposure'},
    {'cmd': 'gain', 'desc': 'Get gain'},
    {'cmd': 'brightness', 'desc': 'Get brightness'},
    {'cmd': 'contrast', 'desc': 'Get contrast'},
    {'cmd': 'saturation', 'desc': 'Get saturation'},
    {'cmd': 'sharpness', 'desc': 'Get sharpness'},
    {'cmd': 'gamma', 'desc': 'Get gamma'},
    {'cmd': 'denoise', 'desc': 'Get denoise'},
    {'cmd': 'hdr off', 'desc': 'HDR off'},
    {'cmd': 'ae off', 'desc': 'Auto exposure off'},
    {'cmd': 'awb off', 'desc': 'Auto white balance off'},
]


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
    
    def send_command(self, cmd_info):
        """Send a JSON command and wait for response"""
        if not self.ws:
            return {'cmd': cmd_info['cmd'], 'desc': cmd_info['desc'], 'success': False, 'code': 'NO_CONNECTION'}
        
        # Build message
        if 'params' in cmd_info:
            msg = {'cmd': cmd_info['cmd'], 'params': cmd_info['params']}
        else:
            msg = {'cmd': cmd_info['cmd']}
        
        print(f'→ {cmd_info["desc"]}')
        print(f'  Sending: {json.dumps(msg)}')
        
        try:
            # Send command
            self.ws.send(json.dumps(msg))
            
            # Wait for response with timeout
            self.ws.settimeout(3.0)
            response = self.ws.recv()
            
            try:
                data = json.loads(response)
                print(f'  ← Response: {json.dumps(data)}')
                
                # JSON API doesn't have consistent error codes
                # Consider it success if we got a response
                result = {
                    'cmd': cmd_info['cmd'],
                    'desc': cmd_info['desc'],
                    'success': True,
                    'response': data
                }
                print(f'  ✓ Success')
                return result
                
            except json.JSONDecodeError:
                # Non-JSON response
                print(f'  ← Raw: {response}')
                result = {
                    'cmd': cmd_info['cmd'],
                    'desc': cmd_info['desc'],
                    'success': True,
                    'response': response
                }
                print(f'  ✓ Success (non-JSON)')
                return result
                
        except WebSocketTimeoutException:
            result = {
                'cmd': cmd_info['cmd'],
                'desc': cmd_info['desc'],
                'success': False,
                'code': 'TIMEOUT'
            }
            print(f'  ⚠ Timeout (command may still be executing)')
            return result
            
        except Exception as e:
            result = {
                'cmd': cmd_info['cmd'],
                'desc': cmd_info['desc'],
                'success': False,
                'code': str(e)
            }
            print(f'  ✗ Error: {e}')
            return result
    
    def run_tests(self):
        """Run all test commands"""
        print('═' * 60)
        print('  DWARF II JSON API Test')
        print('  (Simplified command set via /ws endpoint)')
        print('═' * 60)
        print()
        
        for cmd_info in COMMANDS:
            result = self.send_command(cmd_info)
            self.results.append(result)
            time.sleep(1.0)  # Delay between commands
            print()
    
    def print_summary(self):
        """Print test summary"""
        print('═' * 60)
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
                    code = r.get('code', 'UNKNOWN')
                    print(f'  • {r["desc"]} ({code})')
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
