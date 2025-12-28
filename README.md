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

### Linux

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

### macOS

**Prerequisites:**
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake`
- Qt 6 (including Multimedia, WebSockets components)
- Protobuf: `brew install protobuf`

**Qt6 Installation:**

Install Qt via Qt Online Installer to `~/Qt/` or use Homebrew:

```bash
brew install qt@6
```

**Build:**

```bash
./build_macos_release.sh
```

The script will:
- Auto-detect Qt6 in `~/Qt/6.*/` or `~/Qt/6.*/macos/`
- Auto-detect Protobuf via Homebrew or system installation
- Configure and build the project with CMake
- Create an app bundle in `dist/macos/DwarfController.app`
- Bundle Qt frameworks with `macdeployqt`
- Optionally create a DMG in `dist/zwergII-macos.dmg`

**Environment Variables (optional):**

You can override default paths by setting these before running the build script:

```bash
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos
export Qt6_DIR=/path/to/Qt/6.x.x/macos/lib/cmake/Qt6
export PROTOC_PREFIX_PATH=/usr/local
export Protobuf_DIR=/usr/local/lib/cmake/protobuf
```

**Note:** If Qt6 Multimedia component is missing, install it via Qt Maintenance Tool:
```bash
~/Qt/MaintenanceTool
```

### Windows

**Prerequisites:**
- CMake >= 3.16
- MinGW-w64 (g++) or MSVC compiler
- Qt 6 (including WebSockets component)
- Protobuf compiler (`protoc.exe`) and libraries

**Protobuf Setup (required for Windows):**

The build script can automatically build Protobuf from source. Clone the Protobuf repository:

```bat
cd G:\Download
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git checkout v21.12
```

**Build:**

```bat
build_windows_release.bat
```

The script will:
- Auto-detect vcpkg (if available)
- Auto-detect or build Protobuf from source (if in `G:\Download\protobuf`)
- Configure and build the project with CMake
- Create a distributable package in `dist\windows\`

**Environment Variables (optional):**

You can override default paths by setting these before running the build script:

```bat
set Qt6_DIR=C:\Qt\6.10.1\mingw_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Qt\6.10.1\mingw_64
set PROTOC_PREFIX_PATH=G:\Download\protoc
set Protobuf_DIR=G:\Download\protobuf-install\lib\cmake\protobuf
set CMAKE_TOOLCHAIN_FILE=G:\programming\vcpkg\scripts\buildsystems\vcpkg.cmake
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
