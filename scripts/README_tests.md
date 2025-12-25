# DWARF II Test Scripts

## Available Scripts

### 1. JSON API Test (Simplified)
`test_json_api.py` - Tests undocumented JSON API via `/ws` endpoint

```bash
DWARF_IP=192.168.88.1 python3 test_json_api.py
```

**Commands tested:**
- Camera control: turnOnCamera, takePhoto, video, timelapse, panorama
- ISP settings: exposure, gain, brightness, contrast, saturation, etc.

**Limitations:** Only tests ~20 commands supported by simplified JSON API.

### 2. Protobuf API Test (Complete)
`test_protobuf_api.py` - Tests official Protobuf API

```bash
DWARF_IP=192.168.88.1 python3 test_protobuf_api.py
```

**Commands tested:**
- Telephoto camera: open, params, ISP, capture, close
- Wide camera: open, params, ISP, capture, close
- Focus: auto focus
- Astronomy: dark frame queries
- RGB & Power: light control

**Note:** Does NOT include firmware update or destructive operations.

### 3. Working Panorama Tests (Node.js)
- `test_pano_6x5_polling.js` - JSON API panorama with polling
- `test_pano_protobuf.js` - Protobuf API panorama test

## Requirements

### Python Scripts
- Python 3.12
- `websocket-client` package
- `protobuf` package

### Node.js Scripts
- Node.js 24
- `ws` package
- `protobufjs` package (for Protobuf test)

All dependencies are pre-installed in the container.

## API Endpoints

- **JSON API**: `ws://IP:9900/ws` (undocumented, limited commands)
- **Protobuf API**: `ws://IP:9900` (official, complete command set)

## Notes

- JSON API uses simple command strings: `{"cmd": "takePhoto"}`
- Protobuf API uses WsPacket structure with module_id and cmd numbers
- Default timeout: 3 seconds per command
- Tests run sequentially with delays to avoid overwhelming device
