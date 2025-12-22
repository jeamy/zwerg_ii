# GUI Design Plan - Dwarf II Controller

## Overview
The goal is to modernize the UI to be more "Camera-Centric", prioritizing the live stream while providing intuitive, high-tech controls through a sidebar and semi-transparent overlays.

## Mockup
The conceptual design can be found at `doc/gui_mockup.png`.

## Key Features

### 1. Camera-Centric Layout
- **Fullscreen Live View:** The main camera stream (Tele or Wide) occupies the entire background of the application window.
- **Floating Controls:** UI elements like status badges and the joystick will be semi-transparent overlays on top of the video stream.

### 2. Sidebar Navigation
A vertical icon-based rail on the left side:
- **Connect:** Connection & Discovery dashboard.
- **Camera:** Shooting controls (Photo, Video, Burst).
- **Astro:** Star map, GoTo, and Stacking workflow.
- **Panorama:** Grid-based mosaic capture (New).
- **Gallery:** Full-screen media browser and downloader.
- **Settings:** App and Device configuration.

### 3. Interactive Stream Controls
- **Draggable PiP (Picture-in-Picture):** The secondary stream view can be dragged anywhere within the main view.
- **Stream Swap:** Double-clicking the PiP window swaps the Tele and Wide views.
- **Click-to-Center:** Clicking a point in the main stream sends a centering command to the motors.

### 4. Modular Functional Panels
- Panels like Camera Settings or Astro Navigation will slide in from the side or appear as modal overlays to maintain focus on the live stream.

## Implementation Roadmap

### Phase 1: Core Layout Refactoring (Current)
- [ ] Replace `QTabWidget` with a custom sidebar + `QStackedWidget`.
- [ ] Implement the background-fill video viewport.
- [ ] Create the draggable container for the PiP view.

### Phase 2: Functional Modules
- [ ] Refactor `CameraSettingsPanel` and `AstroNavigationPanel` to fit the new design.
- [ ] Implement the `PanoramaPanel`.
- [ ] Beautify icons and add glassmorphism styles via CSS (Qt Stylesheets).

### Phase 3: Visual Polish
- [ ] Add smooth transitions between panels.
- [ ] Refine the "Connected" status overlays.
- [ ] Implement the semi-transparent floating joystick.
