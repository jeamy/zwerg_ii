# zwergII (DWARF II Controller)

> **⚠️ BETA VERSION - USE AT YOUR OWN RISK**
>
> This software is currently in **beta stage** and under active development. While we strive for stability and reliability, the application may contain bugs or incomplete features. Use this software at your own risk. The developers are not responsible for any damage to your equipment or data loss that may occur from using this application.

Desktop application for controlling the **DWARF II smart telescope**.

This project implements communication with the device via **WebSocket (Protobuf)** and **HTTP**, provides a modern Qt UI client, and includes extensive protocol/API documentation in this repository.

## Features

- **Connection & Discovery**
  - Network scan / device finder
  - WebSocket connection (DWARF II: port 9900)
  - HTTP device requests (DWARF II: port 8082)
- **Camera**
  - Tele and wide camera control
  - Photo, burst, timelapse, and Tele video capture
  - Camera parameters such as exposure, gain, white balance, IR-cut, and image tuning
- **Live view / Streams**
  - MJPEG/JPG live streams (`mainstream` / `secondstream`)
  - Picture-in-picture (PiP) and multi-view UI
- **Astro / Navigation**
  - Go Live, calibration, one-click GOTO, and astro control
  - Live stacking, wide stacking, dark-frame capture, EQ solving
  - Sun/Moon tracking, star map, and local catalog overlay
  - LX200 server integration for external planetarium software
- **Motor / Focus**
  - Virtual joystick
  - Fixed-angle moves and dual-camera linkage
  - Focus control including autofocus, continuous focus, and astro autofocus
  - Live tracking overlay for object tracking and related tracking modes
  - Panorama capture workflow
- **Media/Gallery**
  - Media lists, thumbnails, lightbox view
  - Download via FTP 
- **Other**
  - Settings for time, timezone, MTP, CPU mode, RGB ring, power indicator, shutdown, and reboot
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
  - Widgets, Network, Multimedia, MultimediaWidgets, WebSockets, Sql, Svg
- **Protocol Buffers** (Compiler + C++ libs)

Note: System-specific installation instructions can be found in [`DEVELOPMENT.md`](DEVELOPMENT.md).

## Build

### Linux

**Prerequisites:**
- CMake >= 3.16
- g++ or clang++ with C++17 support
- Qt 6 (including Multimedia, WebSockets, Svg components)
- Protobuf compiler and libraries
- `pkg-config`
- Development libraries: `libgl1-mesa-dev`, `libdbus-1-dev`, `libfreetype6-dev`, `libfontconfig1-dev`, `libxkbcommon-dev`, `libvulkan-dev`

**Install dependencies (Ubuntu 22.04 / Debian-based):**

```bash
sudo apt-get install build-essential cmake git \
  pkg-config protobuf-compiler libprotobuf-dev patchelf zip \
  qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools \
  qt6-multimedia-dev libqt6websockets6-dev libqt6svg6-dev \
  libqt6sql6-sqlite \
  libgl1-mesa-dev libdbus-1-dev libfreetype6-dev \
  libfontconfig1-dev libxkbcommon-dev libvulkan-dev \
  libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 \
  libxcb-randr0 libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 \
  libxcb-cursor0 libxcb-xkb1
```

**Build (Development):**

```bash
./build.sh
```

Or use plain CMake:

```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**Build (Release, native):**

```bash
./build_linux_release.sh
```

This script will:
- Build a Release binary
- Create `dist/linux/`
- Copy runtime assets (`styles/`, `i18n/`, `data/`, optional `resources/`)
- Bundle Qt either via `linuxdeployqt` or `qtpaths6 + patchelf` if available

**Build (Release with Docker / Ubuntu 20.04):**

For maximum compatibility (glibc 2.31), build in an Ubuntu 20.04 Docker container:

```bash
./build_linux_release_docker_ubuntu2004.sh
```

This will:
- Build a Docker image with Qt 6.5.3 and all dependencies
- Compile the project inside the container
- Bundle all required libraries for portability
- Create a distributable package in `dist/linux/`
- Create a ZIP archive `dist/zwergII-linux-release.zip`

**Requirements for Docker build:**
- Docker installed and running
- User must have permission to run Docker commands

**Skip Docker image rebuild (if already built):**

```bash
./build_linux_release_docker_ubuntu2004.sh --skip-build
```

### macOS

**Prerequisites:**
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake`
- Qt 6 (including Multimedia, WebSockets, Svg components)
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
- MSVC compiler recommended
- Ninja recommended for faster builds
- Qt 6 (`msvc2022_64`, including WebSockets, Svg components)
- Protobuf compiler (`protoc.exe`) and libraries
- `windeployqt` recommended for full Qt runtime deployment

**Recommended setup:**

- Use a Developer Command Prompt for Visual Studio 2022, or another shell with `cl.exe` available
- Install Qt `msvc2022_64`
- Install Protobuf through `vcpkg`

Example:

```bat
vcpkg install protobuf:x64-windows
```

The build script prefers:
- `MSVC + Ninja`
- `vcpkg` via `CMAKE_TOOLCHAIN_FILE`
- `Protobuf_DIR` from the active vcpkg triplet if provided

Source-build fallback for Protobuf still exists for older local setups, but it is no longer the recommended path.

**Build:**

```bat
build_windows_release.bat
```

The script will:
- Prefer `MSVC + Ninja` if available
- Auto-detect `vcpkg` and use `CMAKE_TOOLCHAIN_FILE`
- Use `windeployqt` if available, otherwise fall back to a minimal manual Qt copy
- Create a distributable package in `dist\windows\`

**Environment Variables (optional):**

```bat
set Qt6_DIR=C:\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Qt\6.10.1\msvc2022_64
set Qt6WebSockets_DIR=C:\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6WebSockets
set PROTOC_PREFIX_PATH=C:\path\to\protoc
set CMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
set Protobuf_DIR=C:\path\to\vcpkg\installed\x64-windows\share\protobuf
set VCPKG_TARGET_TRIPLET=x64-windows
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

- **API/Protocol (DWARF II)**: [`docs/DWARF_II_API_COMPLETE.md`](docs/DWARF_II_API_COMPLETE.md)
- **Development setup**: [`DEVELOPMENT.md`](DEVELOPMENT.md)

## Notes

- Depending on the mode, the device uses different stream and API URLs. Details: [`docs/DWARF_II_API_COMPLETE.md`](docs/DWARF_II_API_COMPLETE.md).
- This repository contains a `legacy/` folder with an earlier web/backend implementation; the current codebase is the Qt/C++ application.

## License

MIT License. See [`LICENSE`](LICENSE).

## Release Build Scripts

This repository includes cross-platform release build scripts

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
  - Prefers `MSVC + Ninja`
  - Uses `windeployqt` if available (fallback to a minimal manual Qt copy otherwise)
  - Supports `vcpkg`-based Protobuf/toolchain setups

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

## GitHub Releases

This repository includes a manual GitHub Actions release workflow:

- Workflow: [`.github/workflows/manual-release.yml`](.github/workflows/manual-release.yml)
- Trigger: manual only (`workflow_dispatch`)
- Inputs:
  - `tag` (required)
  - `ref` (default: `master`)
- Behavior:
  - creates or reuses the requested tag
  - automatically retags if the tag already exists but is not yet used by a GitHub Release
  - builds:
    - Linux on Ubuntu 22.04
    - macOS Intel
    - macOS Apple Silicon
    - Windows
  - uploads the generated ZIP archives directly to the GitHub Release

### Acknowledgments

This project implements the DWARF II API based on reverse-engineered protocol documentation. Special thanks to the DWARF Lab community for sharing API information:
https://github.com/DwarfTelescopeUsers/dwarfii_api

**Development:** Vibe coding with support from Windsurf and various AI models, enabling rapid prototyping and implementation of the DWARF II protocol.

**Testing:** This project was tested on Windows, macOS and Linux.
