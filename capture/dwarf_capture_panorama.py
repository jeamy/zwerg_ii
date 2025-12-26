#!/usr/bin/env python3
"""
Dedizierter Test für DWARF II Panorama Row/Col Einstellungen
Sendet Feature Param Commands und analysiert die Antworten
"""

import sys
import json
import time
import struct
from datetime import datetime, timezone
from websocket import WebSocket, create_connection

sys.path.insert(0, '/media/data/programming/zwergII/capture')
import dwarf_proto_runtime as proto


def varint_encode(n: int) -> bytes:
    """Encode integer as protobuf varint"""
    result = bytearray()
    while n > 0x7F:
        result.append((n & 0x7F) | 0x80)
        n >>= 7
    result.append(n & 0x7F)
    return bytes(result)


def varint_decode(data: bytes, offset: int = 0) -> tuple[int, int]:
    """Decode protobuf varint, return (value, new_offset)"""
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


def build_ws_packet(module_id: int, cmd: int, payload: bytes = b"") -> bytes:
    """Build WsPacket protobuf message"""
    packet = proto.WsPacket()
    packet.module_id = module_id
    packet.cmd = cmd
    packet.type = 0
    if payload:
        packet.data = payload
    return packet.SerializeToString()


def send_command(ws: WebSocket, module_id: int, cmd: int, payload: bytes = b"", 
                 expect_response: bool = True, timeout: float = 5.0) -> dict:
    """Send command and optionally wait for response"""
    packet = build_ws_packet(module_id, cmd, payload)
    
    # Prepend length
    msg = varint_encode(len(packet)) + packet
    
    print(f">>> Sending: module={module_id}, cmd={cmd}, payload={len(payload)} bytes")
    if payload:
        print(f"    Payload hex: {payload.hex()}")
    
    try:
        ws.send(msg, opcode=0x2)
        print(f"    Sent successfully")
    except Exception as e:
        print(f"    Send failed: {e}")
        return {"error": f"send_failed: {e}"}
    
    if not expect_response:
        print(f"    Not expecting response (fire-and-forget)")
        return {"sent": True}
    
    # Wait for response
    print(f"    Waiting for response (timeout={timeout}s)...")
    start = time.time()
    responses_seen = 0
    while time.time() - start < timeout:
        try:
            raw = ws.recv()
            if not raw:
                continue
            responses_seen += 1
            
            # Decode length prefix
            pkt_len, offset = varint_decode(raw)
            packet_data = raw[offset:offset + pkt_len]
            
            # Parse WsPacket
            res_packet = proto.WsPacket()
            res_packet.ParseFromString(packet_data)
            
            # Log all packets
            print(f"<<< Received: module={res_packet.module_id}, cmd={res_packet.cmd}, type={res_packet.type}")
            
            # Skip notifications (type=0) but count them
            if res_packet.type == 0:
                print(f"    (Notification, skipping)")
                continue
            
            # Check if this is our response
            if res_packet.module_id == module_id and res_packet.cmd == cmd:
                print(f"<<< Response: module={res_packet.module_id}, cmd={res_packet.cmd}, "
                      f"type={res_packet.type}, data={len(res_packet.data)} bytes")
                
                # Try to parse as ComResponse
                result = {
                    "module_id": res_packet.module_id,
                    "cmd": res_packet.cmd,
                    "type": res_packet.type,
                    "data_hex": res_packet.data.hex() if res_packet.data else "",
                }
                
                if res_packet.data:
                    try:
                        com_resp = proto.ComResponse()
                        com_resp.ParseFromString(res_packet.data)
                        result["code"] = com_resp.code
                        print(f"    ComResponse.code = {com_resp.code}")
                    except Exception as e:
                        print(f"    Failed to parse as ComResponse: {e}")
                
                return result
        except Exception as e:
            print(f"    Error receiving: {e}")
            import traceback
            traceback.print_exc()
            break
    
    print(f"    Timeout after {timeout}s ({responses_seen} packets received)")
    return {"error": "timeout", "packets_received": responses_seen}


def wait_for_panorama_notifications(ws: WebSocket, timeout: float = 300.0) -> list:
    """Wait for panorama progress notifications (cmd 15219)"""
    print(f"\n>>> Waiting for panorama progress notifications (timeout={timeout}s)...")
    notifications = []
    start = time.time()
    
    while time.time() - start < timeout:
        try:
            raw = ws.recv()
            if not raw:
                continue
            
            pkt_len, offset = varint_decode(raw)
            packet_data = raw[offset:offset + pkt_len]
            
            res_packet = proto.WsPacket()
            res_packet.ParseFromString(packet_data)
            
            # Panorama progress notification: module=10, cmd=15219
            if res_packet.module_id == 10 and res_packet.cmd == 15219:
                # Parse payload
                if res_packet.data:
                    # Expected format: field 1 (current), field 2 (total)
                    data = res_packet.data
                    try:
                        current = None
                        total = None
                        off = 0
                        while off < len(data):
                            tag, off = varint_decode(data, off)
                            field_num = tag >> 3
                            wire_type = tag & 0x7
                            
                            if wire_type == 0:  # Varint
                                value, off = varint_decode(data, off)
                                if field_num == 1:
                                    current = value
                                elif field_num == 2:
                                    total = value
                        
                        if current is not None and total is not None:
                            print(f"<<< Panorama Progress: {current}/{total}")
                            notifications.append({"current": current, "total": total})
                            
                            # Check if complete
                            if current >= total:
                                print(f">>> Panorama complete!")
                                return notifications
                    except Exception as e:
                        print(f"    Failed to parse notification: {e}")
        except Exception as e:
            print(f"    Error: {e}")
            break
    
    print(f">>> Timeout or error waiting for panorama completion")
    return notifications


def main():
    host = "10.42.0.209"
    port = 9900
    pano_rows = 3
    pano_cols = 4
    
    print("=" * 80)
    print(f"DWARF II Panorama Test")
    print(f"Host: {host}:{port}")
    print(f"Requested Grid: {pano_rows} rows x {pano_cols} cols = {pano_rows * pano_cols} total")
    print("=" * 80)
    print()
    
    log = {
        "meta": {
            "host": host,
            "started_at": datetime.now(timezone.utc).isoformat(),
            "requested_rows": pano_rows,
            "requested_cols": pano_cols,
            "requested_total": pano_rows * pano_cols,
        },
        "commands": [],
        "notifications": [],
    }
    
    try:
        # Connect
        print(f">>> Connecting to ws://{host}:{port}...")
        ws = create_connection(f"ws://{host}:{port}", timeout=5.0)
        print(f">>> Connected!\n")
        
        # 1. Panorama UI Open (Module 14, CMD 16402)
        print("STEP 1: Panorama UI Open")
        print("-" * 80)
        result = send_command(ws, 14, 16402, bytes.fromhex("0807"))
        log["commands"].append({"name": "panorama_ui_open", "result": result})
        time.sleep(0.5)
        print()
        
        # 2. Set Panorama Rows (Module 1, CMD 10037, Feature ID 6)
        print("STEP 2: Set Panorama Rows")
        print("-" * 80)
        param_rows = proto.CommonParam(
            id=6,  # FEATURE_ID_PANO_ROW
            mode_index=1,
            continue_value=float(pano_rows)
        )
        req_rows = proto.ReqSetFeatureParams(param=param_rows)
        payload_rows = req_rows.SerializeToString()
        result = send_command(ws, 1, 10037, payload_rows, expect_response=True)
        log["commands"].append({"name": "panorama_set_rows", "result": result})
        time.sleep(0.5)
        print()
        
        # 3. Set Panorama Cols (Module 1, CMD 10037, Feature ID 7)
        print("STEP 3: Set Panorama Cols")
        print("-" * 80)
        param_cols = proto.CommonParam(
            id=7,  # FEATURE_ID_PANO_COL
            mode_index=1,
            continue_value=float(pano_cols)
        )
        req_cols = proto.ReqSetFeatureParams(param=param_cols)
        payload_cols = req_cols.SerializeToString()
        result = send_command(ws, 1, 10037, payload_cols, expect_response=True)
        log["commands"].append({"name": "panorama_set_cols", "result": result})
        time.sleep(0.5)
        print()
        
        # 4. Start Panorama (Module 10, CMD 15500)
        print("STEP 4: Start Panorama Grid")
        print("-" * 80)
        req_start = proto.ReqStartPanoramaByGrid()
        payload_start = req_start.SerializeToString()
        result = send_command(ws, 10, 15500, payload_start, expect_response=False)
        log["commands"].append({"name": "panorama_start_grid", "result": result})
        time.sleep(1.0)
        print()
        
        # 5. Wait for panorama progress notifications
        print("STEP 5: Monitor Panorama Progress")
        print("-" * 80)
        notifications = wait_for_panorama_notifications(ws, timeout=300.0)
        log["notifications"] = notifications
        
        # Analysis
        print()
        print("=" * 80)
        print("ANALYSIS")
        print("=" * 80)
        
        if notifications:
            actual_total = notifications[0]["total"]
            print(f"Requested: {pano_rows} x {pano_cols} = {pano_rows * pano_cols} images")
            print(f"Actual:    {actual_total} images")
            print()
            
            if actual_total == pano_rows * pano_cols:
                print("✓ SUCCESS: Panorama grid settings were applied correctly!")
                log["meta"]["success"] = True
            else:
                print("✗ FAILURE: DWARF II ignored row/col settings!")
                print(f"  Expected: {pano_rows * pano_cols}, Got: {actual_total}")
                log["meta"]["success"] = False
                
                # Try to guess what grid was used
                if actual_total == 25:
                    print("  → DWARF used default 5x5 grid")
                elif actual_total == 9:
                    print("  → DWARF used 3x3 grid")
                else:
                    print(f"  → Unknown grid size")
        else:
            print("✗ ERROR: No panorama notifications received")
            log["meta"]["success"] = False
        
        ws.close()
        
    except Exception as e:
        print(f"\n✗ ERROR: {e}")
        import traceback
        traceback.print_exc()
        log["meta"]["error"] = str(e)
    
    finally:
        log["meta"]["finished_at"] = datetime.now(timezone.utc).isoformat()
        
        # Save log
        log_file = f"/media/data/programming/zwergII/capture/pano_test_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(log_file, "w") as f:
            json.dump(log, f, indent=2)
        print()
        print(f"Log saved to: {log_file}")


if __name__ == "__main__":
    main()
