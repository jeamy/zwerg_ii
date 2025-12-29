# Development Environment Setup

> **Note:** This project is fully functional and in beta testing phase. All major features are implemented.

To compile and develop the DWARF II Qt application, the following dependencies must be installed on your system.

## 1. System Requirements
*   **Operating System:** Linux (Ubuntu 20.04+, Debian 11+, Fedora 35+), Windows 10/11, or macOS 10.15+
*   **Compiler:** C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+, MinGW-w64)
*   **Build System:** CMake 3.16 or newer

## 2. Required Libraries

### Qt 6
We use **Qt 6** (minimum 6.2 LTS, recommended 6.5+).
Required modules:
*   `qt6-base` (Core, Gui, Widgets, Network, Sql)
*   `qt6-multimedia` (Video streaming, QMediaPlayer, QVideoWidget)
*   `qt6-websockets` (WebSocket communication with DWARF II)

### Protocol Buffers
For communication with the telescope.
*   `protobuf-compiler` (protoc)
*   `libprotobuf-dev` (C++ bindings)
*   Version 3.x or 21.x recommended

## 3. Installation Instructions

### Fedora Linux

Many Qt libraries are already present on Fedora (especially with KDE Plasma). For development, we need the header files (`-devel`).

```bash
sudo dnf install -y \
    cmake \
    gcc-c++ \
    qt6-qtbase-devel \
    qt6-qtmultimedia-devel \
    qt6-qtwebsockets-devel \
    protobuf-devel \
    protobuf-compiler
```

### Debian / Ubuntu

```bash
sudo apt-get install -y \
    build-essential cmake git \
    qt6-base-dev qt6-multimedia-dev qt6-websockets-dev \
    protobuf-compiler libprotobuf-dev \
    libgl1-mesa-dev libdbus-1-dev libfreetype6-dev \
    libfontconfig1-dev libxkbcommon-dev libvulkan-dev
```

### Windows

**Option 1: Qt Online Installer (Recommended)**
1.  Install the [Qt Online Installer](https://www.qt.io/download-qt-installer)
2.  Select **Qt 6.5+** with **MinGW** or **MSVC** compiler
3.  Ensure these components are selected:
    *   Qt Multimedia
    *   Qt WebSockets
4.  Install CMake from https://cmake.org/download/

**Option 2: Protobuf Setup**
*   Clone and build from source (see `README.md` Windows build section)
*   Or use vcpkg: `vcpkg install protobuf:x64-mingw-static`

**Build:**
```bat
build_windows_release.bat
```

### macOS

**Install dependencies:**
```bash
brew install cmake protobuf
```

**Install Qt6:**
*   Via Qt Online Installer to `~/Qt/` (recommended)
*   Or via Homebrew: `brew install qt@6`

**Build:**
```bash
./build_macos_release.sh
```

## 4. Build Instructions

### Quick Development Build (Linux)

```bash
./build.sh
```

Or manually:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./DwarfController
```

### Release Builds

See `README.md` for detailed release build instructions for each platform:
*   **Linux**: `./build_linux_release_docker_ubuntu2004.sh` (Docker-based for maximum compatibility)
*   **macOS**: `./build_macos_release.sh` (creates app bundle and DMG)
*   **Windows**: `build_windows_release.bat` (creates distributable ZIP)

## 5. Project Structure

```
zwergII/
├── src/
│   ├── net/              # Network layer (WebSocket, HTTP, Controllers)
│   ├── ui/               # UI panels (Camera, Astro, Panorama, etc.)
│   ├── proto/            # Protobuf definitions
│   ├── MainWindow.cpp    # Main application window
│   └── main.cpp
├── data/                 # Star catalog, constellation data
├── styles/               # Qt stylesheets (dark theme)
├── i18n/                 # Translations (German, English)
├── resources/            # Icons (SVG)
├── docs/                 # API documentation
└── CMakeLists.txt
```

## 6. Development Notes

*   **Internationalization**: All UI strings use `tr()` for translation support
*   **Dark Theme**: Application uses custom QSS stylesheet (`styles/app.qss`)
*   **Protocol**: WebSocket communication via Protobuf on port 9900
*   **Streaming**: MJPEG streams via HTTP on port 8092
*   **Architecture**: Signal/Slot based with separate controller classes for each module
