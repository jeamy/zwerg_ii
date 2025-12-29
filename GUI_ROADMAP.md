# GUI Design & Implementation Status

> **Status:** ✅ **FULLY IMPLEMENTED** - All features described in this document are now functional in the beta release.

This document describes the visual design and user interface (GUI) of the DWARF II Qt application. The goal is a professional, "cockpit"-like interface that groups all functions logically and is optimized for night use.

## 1. Design Philosophy (Implemented)

*   **Dark Mode First:** ✅ The entire application uses a dark color scheme (#2D2D30) to preserve night vision during astrophotography.
*   **Modular Layout:** ✅ Sidebar-based design with collapsible panels. Fixed layout optimized for telescope control.
*   **Focus on the Image:** ✅ Live stream takes maximum available space with Picture-in-Picture (PiP) support.
*   **Visual Feedback:** ✅ Active states (e.g., "Recording", "Connected") are highlighted with clear signal colors (Orange/Red).

## 2. Layout Structure (Implemented)

The main window (`QMainWindow`) is divided into three areas:

### A. Central Area (Viewport) ✅
*   **Component:** `QLabel` with MJPEG stream rendering
*   **Content:** Displays live stream (Tele or Wide camera)
*   **Picture-in-Picture:** ✅ Secondary camera view in corner (toggleable)
*   **Overlays:** ✅ Transparent overlays for motor control, camera parameters, and star map

### B. Left Sidebar (Navigation) ✅
*   **Component:** Vertical button group with icons
*   **Width:** Fixed (65px), always visible when connected
*   **Tabs:**
    *   📷 **CAM** - Camera control and settings
    *   🔭 **ASTRO** - Astrophotography and GOTO navigation
    *   🗺️ **PANO** - Panorama mode
    *   🖼️ **GAL** - Gallery browser
    *   ⚙️ **SET** - Settings and system info

### C. Right Panel (Control Deck) ✅
*   **Component:** `QStackedWidget` with fixed width (350px)
*   **Content:** Context-sensitive controls for selected sidebar tab
*   **Features:** Collapsible overlays for motor joystick and camera parameters

### D. Status Bar ✅
*   **Component:** `QStatusBar`
*   **Content:** Connection status, device info, telemetry

---

## 3. Function Modules (Implemented)

### Tab 1: 📷 Camera & Capture ✅

**Implemented Features:**
*   ✅ **Stream Source:** Toggle between TELE and WIDE cameras
*   ✅ **Capture Controls:**
    *   Photo capture button (both cameras)
    *   Video recording button (Tele camera only)
    *   Recording indicator with timer
*   ✅ **Exposure Control:**
    *   Exposure time slider with preset values
    *   Gain/ISO slider with preset values
    *   Real-time parameter updates
*   ✅ **Image Parameters:**
    *   IR-Cut filter toggle
    *   Brightness, Contrast, Hue, Saturation, Sharpness sliders
    *   White balance control
    *   Format selection (RAW/FITS/TIFF)
*   ✅ **Camera Parameter Overlay:** Floating panel with quick access to all settings

### Tab 2: 🔭 Astro & Navigation ✅

**Implemented Features:**
*   ✅ **Star Map Overlay:**
    *   Custom `StarMapWidget` with OpenGL rendering
    *   Integrated SQLite database (HYG Star Catalog + NGC catalog)
    *   Real-time sky view based on location and time
    *   Constellation lines and labels
    *   Interactive object selection
*   ✅ **GOTO Functionality:**
    *   Click on celestial object to slew telescope
    *   Manual RA/Dec coordinate input
    *   Target tracking and position updates
    *   Calibration workflow
*   ✅ **Live Stacking:**
    *   Start/Stop stacking controls
    *   Exposure and gain settings for astro mode
    *   Real-time progress monitoring (frame count, total exposure)
    *   Stacking state notifications
*   ✅ **Object Database:**
    *   NGC deep-sky objects
    *   Searchable catalog
    *   Object information display

### Tab 3: 🗺️ Panorama ✅

**Implemented Features:**
*   ✅ **Panorama Capture:** Start/stop panorama mode
*   ✅ **Progress Monitoring:** Real-time panorama capture progress
*   ✅ **Parameter Control:** Panorama settings and configuration

**Motor Control (Overlay):** ✅
*   ✅ **Virtual Joystick:** Floating overlay with directional controls
*   ✅ **Speed Control:** Variable slew speed adjustment
*   ✅ **Manual Positioning:** Fine control over azimuth and altitude

**Focus Control:** ✅
*   ✅ **Manual Focus:** Step-by-step focus adjustment
*   ✅ **Focus Position:** Numerical focus value display
*   ✅ **Focus Commands:** Multiple step sizes for coarse/fine adjustment

### Tab 4: 🖼️ Gallery ✅

**Implemented Features:**
*   ✅ **Media Browser:** View captured photos and videos
*   ✅ **Thumbnail Grid:** Quick preview of all images
*   ✅ **Image Management:** Access to stored media on DWARF II
*   ✅ **Full-Screen Overlay:** Dedicated gallery view

### Tab 5: ⚙️ Settings & System ✅

**Implemented Features:**
*   ✅ **Connection Management:**
    *   Network scanner for device discovery
    *   Manual IP address input
    *   Connect/Disconnect controls
    *   Connection status monitoring
*   ✅ **Device Information:**
    *   Firmware version display
    *   Battery status
    *   Storage information
*   ✅ **Application Settings:**
    *   Language selection (English/German)
    *   Theme customization
*   ✅ **Advanced Features:**
    *   System information display
    *   Log viewing

---

## 4. Visual Design (Implemented)

The design is inspired by professional creative software (Blender, DaVinci Resolve).

**Color Scheme:** ✅
*   Background (App): `#1E1E1E` (Very dark gray)
*   Background (Panels): `#2D2D30` (Dark gray)
*   Text (Primary): `#E0E0E0` (Light gray, not pure white)
*   Text (Secondary): `#AAAAAA`
*   **Accent Color:** `#FF9800` (Orange) for active states

**Typography:** ✅
*   Font: System default (Segoe UI/Roboto/San Francisco)
*   Size: 10pt (Standard), 12pt (Buttons)

**Icons:** ✅
*   Vector icons (SVG) in white/gray
*   Custom icons for all major functions
*   Located in `resources/icons/`

## 5. Implementation in Qt (Completed)

### Qt Widgets Structure ✅
```cpp
// MainWindow (Implemented)
QMainWindow
├── QWidget (CentralWidget)
│   └── QHBoxLayout
│       ├── QWidget (Sidebar) - Vertical button group
│       ├── QLabel (VideoViewport) - MJPEG stream display
│       │   ├── MotorOverlay (Floating joystick)
│       │   ├── ParamsOverlay (Camera parameters)
│       │   └── StarMapOverlay (Celestial navigation)
│       └── QStackedWidget (RightPanel) - Context panels
│           ├── CameraSettingsPanel
│           ├── AstroNavigationPanel
│           ├── PanoramaPanel
│           ├── GalleryPanel
│           └── SettingsPanel
└── QStatusBar
```

### Network Architecture ✅
*   **WebSocket Client:** `DwarfWebSocketClient` for Protobuf communication (port 9900)
*   **HTTP Client:** `DwarfHttpClient` for REST API (port 8082)
*   **Message Dispatcher:** Routes incoming messages to appropriate controllers
*   **Controllers:**
    *   `DwarfCameraController` - Camera parameters and capture
    *   `DwarfAstroController` - Astrophotography and GOTO
    *   `DwarfMotorController` - Manual telescope movement
    *   `DwarfFocusController` - Focus control
    *   `DwarfPanoramaController` - Panorama mode

### Video Streaming ✅
*   **MJPEG Streaming:** HTTP-based MJPEG streams (port 8092)
*   **Dual Camera Support:** Simultaneous Tele and Wide streams
*   **Picture-in-Picture:** Secondary camera view in corner
*   **Recording:** Native recording to DWARF II SD card via WebSocket commands

## 6. Current Status

**All features described in this document are now implemented and functional in the beta release.**

For build instructions and usage, see `README.md`.
For development setup, see `DEVELOPMENT.md`.
