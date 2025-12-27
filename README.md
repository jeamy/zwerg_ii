# zwergII (DWARF II Controller)

 Desktop application for controlling the **DWARF II smart telescope**.

 This project implements communication with the device via **WebSocket (Protobuf)** and **HTTP**, provides a modern Qt UI client, and includes extensive protocol/API documentation in this repository.

## Features

- **Connection & Discovery**
  - Network scan / device finder
  - WebSocket connection (DWARF II: port 9900)
  - HTTP requests (DWARF II: port 8082)
- **Camera**
  - Tele and wide camera
  - Photo/video capture
  - Camera parameters (exposure/gain/IR-cut, etc.)
- **Live view / Streams**
  - MJPEG/JPG streams (e.g. port 8092, depending on mode)
  - Picture-in-picture (PiP) in the UI
- **Astro / Navigation**
  - GOTO / astro control
  - Star map / overlay (local catalog data)
- **Motor / Focus**
  - Virtual joystick
  - Focus control incl. (astro) autofocus
- **Media/Gallery**
  - Media lists / thumbnails
  - Download (e.g. FTP/MTP, depending on mode)
- **Other**
  - LX200 server (integration for external planetarium software)
  - i18n (German/English)

## Technology Stack

- **C++**: C++17
- **UI**: Qt 6 (Widgets)
- **Networking**: Qt Network, Qt WebSockets
- **Multimedia/Rendering**: Qt Multimedia (depends on streaming mode)
- **Protocol**: Protocol Buffers (Protobuf)
- **Build**: CMake

## Repository Structure (Excerpt)

- `src/`
  - `main.cpp`, `MainWindow.*` (Qt app entry point + main window)
  - `net/` (WebSocket/HTTP/dispatcher/controllers)
  - `ui/` (panels/overlays/widgets)
  - `proto/` (`.proto` files + generated Protobuf artifacts)
- `capture/` (tcpdump/pcapng captures + scripts/notes for dataflow between camera and app)
- `styles/` (Qt stylesheet `app.qss`)
- `i18n/` (translations `.qm`)
- `docs/` (API/protocol documentation)
- `data/` (catalog data / helper scripts)
- `legacy/` (older backend/frontend implementation)

## Requirements

- **CMake** >= 3.16
- **C++ compiler** with C++17 support
- **Qt 6** (at least 6.2 LTS recommended)
  - Widgets, Network, Multimedia, MultimediaWidgets, WebSockets, Sql
- **Protocol Buffers** (Compiler + C++ libs)

Note: System-specific installation instructions can be found in `DEVELOPMENT.md`.

## Build

There is a simple build script:

```bash
./build.sh
```

Alternatively, use plain CMake:

```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Run

After the build (in the `build/` directory), run the generated binary:

```bash
./build/DwarfController
```

## Usage (Quick)

- **Power on the DWARF II**
- Connect to the **DWARF II Wi-Fi** (typically AP mode)
- In the app, use the device IP (often `192.168.88.1`)
- Connect and use the modules (camera/astro/motor/gallery)

## Documentation

- **API/Protocol (DWARF II)**: `docs/DWARF_II_API_COMPLETE.md`
- **Qt Roadmap**: `QT_ROADMAP.md`
- **GUI/UX Roadmap**: `GUI_ROADMAP.md`
- **Development setup**: `DEVELOPMENT.md`

## Notes

- Depending on the mode, the device uses different stream and API URLs. Details: `docs/DWARF_II_API_COMPLETE.md`.
- This repository contains a `legacy/` folder with an earlier web/backend implementation; the current codebase is the Qt/C++ application.

## License

MIT License. See `LICENSE`.

## Release Build Scripts

This repository includes cross-platform release build scripts (adapted from the `astrouni` workflow):

- `build_linux_release.sh`
  - Builds a Linux Release binary via CMake
  - Creates `dist/linux/` with the executable and required runtime assets (`styles/`, `i18n/`, `data/`)
  - Optionally bundles Qt runtime via `linuxdeployqt` or `qtpaths6 + patchelf` (if available)
- `build_linux_release_docker_ubuntu2004.sh`
  - Builds the same Linux release inside an Ubuntu 20.04 container (glibc 2.31)
  - Uses `docker/ubuntu20.04/Dockerfile`
- `build_macos_release.sh`
  - Builds a macOS `.app` bundle and creates `dist/macos/`
  - Bundles Qt frameworks via `macdeployqt` if available
- `build_windows_release.bat`
  - Builds a Windows Release and creates `dist\\windows\\`
  - Uses `windeployqt` if available (fallback to a minimal manual Qt copy otherwise)

### Usage

Linux:

```bash
./build_linux_release.sh
```

Linux (Docker / Ubuntu 20.04):

```bash
./build_linux_release_docker_ubuntu2004.sh
```

macOS:

```bash
./build_macos_release.sh
```

Windows (Developer Command Prompt / terminal with Qt + compiler in PATH):

```bat
build_windows_release.bat
```
