# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project overview
This repo contains a **Qt 6 / C++17** desktop controller named `DwarfController` (root `CMakeLists.txt`).

Most active development is under `src/`.

## Common commands

### Qt desktop app (CMake)

#### Build (Debug)
Preferred (uses the repo’s script):
```bash
./build.sh
```
Equivalent manual build:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j"$(nproc)"
```

#### Build (Release)
```bash
./build.sh release
```

#### Clean build
```bash
./build.sh clean
```

#### Run
After building, the binary is typically:
```bash
./build/DwarfController
```

#### Clangd / compile_commands.json
`.clangd` points clangd at `build/compile_commands.json`. Generate it via:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j"$(nproc)"
```

### Runtime data (star catalog)
The Qt UI contains a star map that reads `data/stars.db` (SQLite).

## High-level architecture (Qt app)

### Entry point and runtime assets
- `src/main.cpp` creates the `QApplication`, loads:
  - translations from `i18n/` (runtime copy ends up in `build/i18n/` via a CMake post-build copy),
  - the stylesheet `styles/app.qss` (runtime copy ends up in `build/styles/`).
- The main window is `MainWindow` (`src/MainWindow.h`/`.cpp`).

### Network + protocol layering
The DWARF device uses multiple protocols:
- **WebSocket (port 9900)** for command/control with a protobuf envelope.
- **HTTP (port 8082)** for device/album endpoints.
- **HTTP MJPEG (port 8092)** for live video streams.
- **FTP (port 21)** for downloading media files and thumbnails.

Key modules:
- `src/net/DwarfWebSocketClient.*`
  - Owns a `QWebSocket` connection to `ws://<ip>:9900`.
  - Wraps payloads in `dwarf::WsPacket` (protobuf from `src/proto/*.proto`) and emits `messageReceived(moduleId, cmd, data)`.
  - Sends a periodic text `ping` (expects `pong`).
- `src/net/DwarfMessageDispatcher.*`
  - Pure routing layer: takes `(moduleId, cmd, data)` and emits module-specific Qt signals like `cameraTeleMessage`, `astroMessage`, `notifyMessage`, etc.
  - `MainWindow` currently wires camera tele/wide messages; other module signals can be connected as features expand.
- `src/net/ProtobufHelper.*`
  - Convenience wrappers for protobuf serialize/parse to/from `QByteArray`.

### Controllers (high-level “actions”)
These classes translate UI actions into DWARF commands (protobuf request messages + command IDs) and optionally parse responses:
- `src/net/DwarfCameraController.*`
  - Provides camera actions (open/close/photo/record) and parameter setters.
  - Maintains cached `ReqSetAllParams` state for Tele/Wide.
  - Parses `ResGetAllParams` responses in `handleCameraMessage(...)` and emits `allParamsReceived(kind)`.
- `src/net/DwarfMotorController.*` / `src/net/DwarfFocusController.*`
  - Wrap motor/focus protobuf requests.
- `src/net/DwarfAstroController.*`
  - Wraps astro workflows (calibration, GOTO, stacking, dark frames, EQ solving).
  - Also parses **notification messages** (module 9) to update UI state like battery/stacking progress.

### Device discovery
- `src/net/DwarfFinder.*` scans a subnet by attempting TCP connects to port **8082** with limited concurrency.
- If port 8082 is open, it tries `http://<ip>:8082/getdeviceInfo` and emits `deviceFound`.

### Video streaming + interaction
- `src/net/DwarfMjpegStream.*` opens an HTTP stream and extracts JPEG frames via SOI/EOI markers.
- `src/net/DwarfMjpegView.*` renders the current `QImage` and emits a normalized double-click coordinate (`pointClicked`) for “click-to-control” features.
- `MainWindow` starts streams after camera open responses:
  - Tele: `http://<ip>:8092/mainstream`
  - Wide: `http://<ip>:8092/secondstream`
  It swaps main/PiP streams on double-click.

### Media gallery (HTTP + FTP)
- `src/net/DwarfHttpClient.*` calls DWARF HTTP endpoints (currently used for listing media; delete is best-effort and firmware-dependent).
- `src/net/DwarfFtpDownloader.*` + `src/net/DwarfFtpClient.*` download files and thumbnails.
  - DWARF HTTP API returns paths like `/sdcard/DWARF_II/...`; the downloader converts to FTP paths by stripping `/sdcard`.

### “Astro & Navigation” UI (star map)
- `src/ui/StarMapWidget.*` is a `QGraphicsView` that:
  - loads stars/DSOs from a SQLite DB (`data/stars.db`) using Qt SQL,
  - computes visibility (alt/az) from observer location + time.
- `src/ui/AstroNavigationPanel.*` hosts the star map, search, stacking controls, and location settings.

