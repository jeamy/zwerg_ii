# Configuration System

## Overview

`zwergII` uses a centralized JSON-based configuration system for persistent UI and runtime settings.

- File format: JSON
- File name: `config.json`
- File location: next to the executable via `QCoreApplication::applicationDirPath()`
- Core implementation: `src/AppConfig.h`, `src/AppConfig.cpp`

The configuration is loaded during application startup and written back when panels or the main window save their state.

## Core Behavior

`AppConfig` is a process-wide singleton with a thread-safe in-memory `QJsonObject`.

Main API:

- `load()`: loads `config.json`
- `save()`: writes `config.json`
- `getValue(section, key, defaultValue)`: reads a single value
- `setValue(section, key, value)`: writes a single value in memory
- `getSection(section)`: returns a whole JSON object section
- `setSection(section, data)`: replaces a whole section
- `configFilePath()`: returns the resolved path to `config.json`

Important detail:

- Missing config file is treated as valid empty state
- Invalid JSON resets the in-memory config to an empty object and `load()` returns `false`
- `setValue()` does not auto-save by itself; save happens when callers explicitly call `save()`

## Load and Save Flow

Main startup:

- `src/main.cpp`: loads the config before the main window is shown
- `src/MainWindow.cpp`: loads persisted UI state into widgets and overlays

Panel-specific persistence:

- `src/ui/CameraSettingsPanel.cpp`: loads and saves camera settings
- `src/ui/AstroNavigationPanel.cpp`: loads and saves astro settings

Main window persistence:

- connection state
- panorama settings
- media download directory
- display/overlay state
- selected language
- last known device metadata

## Active Sections and Keys

The following sections are currently used in the codebase.

### `connection`

Used by `MainWindow`.

Keys:

- `last_ip`
- `subnet`
- `client_mode`
- `last_device_name`
- `last_firmware`
- `firmware_upload_path`

Purpose:

- reconnect convenience
- scan subnet persistence
- view-only mode persistence
- remember last detected device metadata
- remember last selected firmware package for upload

### `camera_tele`

Used by `CameraSettingsPanel` when Tele is active.

Keys:

- `exposure_mode`
- `exposure_index`
- `gain_mode`
- `gain_index`
- `brightness`
- `contrast`
- `saturation`
- `sharpness`
- `hue`
- `ir_cut`
- `wb_mode`
- `wb_temperature`

### `camera_wide`

Used by `CameraSettingsPanel` when Wide is active.

Keys:

- `exposure_mode`
- `exposure_index`
- `gain_mode`
- `gain_index`
- `brightness`
- `contrast`
- `saturation`
- `sharpness`
- `hue`
- `wb_mode`
- `wb_temperature`

Notes:

- `ir_cut` is only used on Tele
- both camera sections are saved independently

### `astro`

Used by `AstroNavigationPanel`.

Keys:

- `magnitude_limit`
- `show_constellations`
- `show_grid`
- `show_labels`
- `latitude`
- `longitude`
- `altitude`
- `stacking_source`
- `num_frames`
- `exposure_index`
- `gain_index`
- `use_dark_frames`
- `dark_frames_count`
- `flat_frames_count`
- `bias_frames_count`
- `lx200_enabled`
- `lx200_port`

Purpose:

- star map display preferences
- observing site
- stacking defaults
- dark/flat/bias counters
- LX200 server settings

### `panorama`

Used by `MainWindow`.

Keys:

- `rows`
- `cols`

### `media`

Used by `MainWindow`.

Keys:

- `download_dir`

### `display`

Used by `MainWindow`.

Keys:

- `last_tab`
- `motor_overlay_visible`
- `params_overlay_visible`
- `starmap_overlay_enabled`
- `gallery_overlay_enabled`
- `main_stream`
- `pip_stream`
- `pip_x`
- `pip_y`

Purpose:

- restore selected tab
- restore overlay visibility
- restore Tele/Wide stream layout
- restore PiP position

### `ui`

Used by `main.cpp` and `MainWindow`.

Keys:

- `language`

Purpose:

- persist selected UI language (`de` / `en`)

## Legacy / Compatibility Notes

There are still compatibility reads for whole-section access in `MainWindow`:

- `camera_settings`
- `astro_settings`

These sections are not the primary live storage anymore. Current code persists camera settings in `camera_tele` and `camera_wide`, and astro settings in `astro`.

## Example `config.json`

```json
{
  "connection": {
    "last_ip": "192.168.88.1",
    "subnet": "192.168.88",
    "client_mode": false,
    "last_device_name": "DWARF-II",
    "last_firmware": "2.4.1",
    "firmware_upload_path": "/home/user/Downloads/dwarf_firmware.zip"
  },
  "camera_tele": {
    "exposure_mode": 1,
    "exposure_index": 10,
    "gain_mode": 1,
    "gain_index": 15,
    "brightness": 0,
    "contrast": 0,
    "saturation": 0,
    "sharpness": 50,
    "hue": 0,
    "ir_cut": false,
    "wb_mode": 0,
    "wb_temperature": 5
  },
  "camera_wide": {
    "exposure_mode": 0,
    "exposure_index": 8,
    "gain_mode": 0,
    "gain_index": 5,
    "brightness": 0,
    "contrast": 0,
    "saturation": 0,
    "sharpness": 50,
    "hue": 0,
    "wb_mode": 0,
    "wb_temperature": 5
  },
  "astro": {
    "magnitude_limit": 6.0,
    "show_constellations": true,
    "show_grid": false,
    "show_labels": true,
    "latitude": 52.52,
    "longitude": 13.405,
    "altitude": 0.0,
    "stacking_source": "tele",
    "num_frames": 100,
    "exposure_index": 10,
    "gain_index": 60,
    "use_dark_frames": true,
    "dark_frames_count": 20,
    "flat_frames_count": 20,
    "bias_frames_count": 20,
    "lx200_enabled": false,
    "lx200_port": 4030
  },
  "panorama": {
    "rows": 3,
    "cols": 3
  },
  "media": {
    "download_dir": "/home/user/Downloads"
  },
  "display": {
    "last_tab": 0,
    "motor_overlay_visible": true,
    "params_overlay_visible": true,
    "starmap_overlay_enabled": false,
    "gallery_overlay_enabled": false,
    "main_stream": 0,
    "pip_stream": 1,
    "pip_x": 120,
    "pip_y": 120
  },
  "ui": {
    "language": "en"
  }
}
```

## Practical Notes

- This is a portable config model: moving the application directory also moves the config if `config.json` stays beside the executable
- Release build scripts copy `config.json` into the distribution when present
- Settings persistence is intentionally simple and transparent; there is no platform-specific registry or `QSettings` backend involved
