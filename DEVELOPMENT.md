# Development Environment Setup

`zwergII` is a Qt/C++ desktop application for controlling the DWARF II telescope via WebSocket (Protobuf) and HTTP.

This document describes the current development setup and build expectations.

## 1. Supported Development Platforms

- Linux: Ubuntu 22.04+ or similar modern Debian/Fedora systems
- Windows: Windows 10/11 with MSVC
- macOS: current Qt 6-capable macOS setups

CI/release targets currently used in the repository:

- Linux: Ubuntu 22.04
- macOS: macOS 15 Intel and macOS 15 Apple Silicon
- Windows: `windows-latest`

## 2. Toolchain Requirements

- CMake 3.16+
- C++17 compiler
- Qt 6
- Protocol Buffers compiler and C++ runtime

Qt modules required by `CMakeLists.txt`:

- Widgets
- Network
- Multimedia
- MultimediaWidgets
- WebSockets
- Sql
- Svg

## 3. Linux Setup

Recommended for current local development:

```bash
sudo apt-get install -y \
  build-essential cmake git pkg-config \
  protobuf-compiler libprotobuf-dev \
  patchelf zip \
  qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools \
  qt6-multimedia-dev libqt6websockets6-dev libqt6svg6-dev \
  libqt6sql6-sqlite \
  libgl1-mesa-dev libdbus-1-dev libfreetype6-dev \
  libfontconfig1-dev libxkbcommon-dev libvulkan-dev \
  libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 \
  libxcb-randr0 libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 \
  libxcb-cursor0 libxcb-xkb1
```

Development build:

```bash
./build.sh
```

Plain CMake build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Native Linux release build:

```bash
./build_linux_release.sh
```

Compatibility-focused Docker release build:

```bash
./build_linux_release_docker_ubuntu2004.sh
```

Notes:

- `build_linux_release.sh` is the current native release path
- the Docker script still targets Ubuntu 20.04 for older glibc compatibility

## 4. macOS Setup

Install base tools:

```bash
brew install cmake protobuf
```

Install Qt 6:

- recommended: Qt Online Installer into `~/Qt/`
- alternative: `brew install qt@6`

Release-style local build:

```bash
./build_macos_release.sh
```

Optional environment overrides:

```bash
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos
export Qt6_DIR=/path/to/Qt/6.x.x/macos/lib/cmake/Qt6
export PROTOC_PREFIX_PATH=/usr/local
export Protobuf_DIR=/usr/local/lib/cmake/protobuf
```

Notes:

- the script auto-detects Qt via `CMAKE_PREFIX_PATH`, `Qt6_DIR`, `qtpaths6`, or `~/Qt/...`
- `macdeployqt` is used when available

## 5. Windows Setup

Recommended local setup:

- Visual Studio 2022 with C++ toolchain
- Developer Command Prompt for VS 2022
- Qt 6 `msvc2022_64`
- Ninja
- vcpkg for Protobuf

Recommended Protobuf install:

```bat
vcpkg install protobuf:x64-windows
```

Recommended environment variables:

```bat
set Qt6_DIR=C:\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Qt\6.10.1\msvc2022_64
set Qt6WebSockets_DIR=C:\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6WebSockets
set PROTOC_PREFIX_PATH=C:\path\to\protoc
set CMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
set Protobuf_DIR=C:\path\to\vcpkg\installed\x64-windows\share\protobuf
set VCPKG_TARGET_TRIPLET=x64-windows
```

Build:

```bat
build_windows_release.bat
```

Current behavior of the script:

- prefers `MSVC + Ninja`
- falls back to MinGW only when MSVC is not available
- supports `vcpkg`-based Protobuf/toolchain setups
- uses `windeployqt` when available

Legacy source-build fallback for Protobuf still exists in the script, but it is no longer the recommended path.

## 6. GitHub Release Workflow

The repository includes a manual multi-platform release workflow:

- file: `.github/workflows/manual-release.yml`
- trigger: manual only
- inputs:
  - `tag`
  - `ref` with default `master`

Behavior:

- creates or reuses the selected tag
- retags automatically when the tag exists but is not yet bound to a GitHub Release
- builds release ZIPs for:
  - Linux
  - macOS Intel
  - macOS Apple Silicon
  - Windows
- uploads all generated ZIPs to the GitHub Release

## 7. Project Structure

```text
zwergII/
├── src/
│   ├── net/              # WebSocket, HTTP, controller layer
│   ├── ui/               # Panels, overlays, widgets
│   ├── proto/            # Protobuf definitions
│   ├── MainWindow.cpp    # Main application window
│   └── main.cpp
├── data/                 # Star catalog and helper data
├── styles/               # Qt stylesheet
├── i18n/                 # Translations
├── resources/            # Icons and bundled assets
├── docs/                 # API and implementation documentation
├── build.sh              # Simple development build helper
├── build_linux_release.sh
├── build_linux_release_docker_ubuntu2004.sh
├── build_macos_release.sh
└── build_windows_release.bat
```

## 8. Development Notes

- Internationalization uses `tr()` and `.qm` translation files
- Runtime configuration is stored in `config.json` next to the executable
- Live view currently uses MJPEG/JPG streams; RTSP is documented but not used by the Qt client
- The codebase uses module-specific controller classes for camera, astro, motor, focus, tracking, system, panorama, and HTTP access

## 9. Related Documents

- `README.md`
- `CONFIG_SYSTEM.md`
- `docs/API_IMPLEMENTIERUNGSANALYSE.md`
