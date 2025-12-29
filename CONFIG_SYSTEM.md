# Configuration System Documentation

## Overview
A centralized JSON-based configuration system has been implemented to persist all GUI settings across application restarts and tab switches.

## Configuration File
- **Location**: `config.json` in the application directory (same folder as the executable)
- **Format**: Human-readable JSON with indentation
- **Auto-save**: Settings are automatically saved when the application closes

## Implementation

### Core Components

#### AppConfig Class (`src/AppConfig.h`, `src/AppConfig.cpp`)
- Singleton pattern for global access via `AppConfig::instance()`
- Methods:
  - `load()`: Load configuration from `config.json`
  - `save()`: Save configuration to `config.json`
  - `getValue(section, key, defaultValue)`: Get a setting value
  - `setValue(section, key, value)`: Set a setting value
  - `getSection(section)`: Get all settings in a section
  - `setSection(section, data)`: Set all settings in a section

### Settings Sections

#### Connection Settings (`connection`)
- `last_ip`: Last connected IP address
- `subnet`: Network subnet for device scanning
- `client_mode`: View-only mode flag

#### Camera Settings (`camera_tele`, `camera_wide`)
Separate sections for Tele and Wide cameras:
- `exposure_mode`: Auto (0) or Manual (1)
- `exposure_index`: Exposure slider position
- `gain_mode`: Auto (0) or Manual (1)
- `gain_index`: Gain slider position
- `brightness`: Brightness value (-100 to 100)
- `contrast`: Contrast value (-100 to 100)
- `saturation`: Saturation value (-100 to 100)
- `sharpness`: Sharpness value (0 to 100)
- `hue`: Hue value (-180 to 180)
- `ir_cut`: IR-Cut filter enabled (Tele only)
- `wb_mode`: White balance mode (Auto/Manual)
- `wb_temperature`: White balance temperature index

#### Panorama Settings (`panorama`)
- `rows`: Grid rows (3-30)
- `cols`: Grid columns (3-60)

#### Media Settings (`media`)
- `download_dir`: Download directory path

#### Astro Settings (`astro_settings`)
- Location (latitude, longitude)
- Stacking parameters (frames, exposure, gain)
- Dark frame usage
- Calibration frame counts

## Usage

### Loading Settings
Settings are automatically loaded at application startup in `MainWindow::loadSettings()`.

Each panel can load its specific settings:
```cpp
CameraSettingsPanel::loadSettings()
AstroNavigationPanel::loadSettings()
```

### Saving Settings
Settings are automatically saved when:
- Application closes (`MainWindow::~MainWindow()`)
- User changes a setting (auto-save on value change)

Manual save:
```cpp
AppConfig::instance()->save();
```

### Accessing Settings
```cpp
AppConfig *cfg = AppConfig::instance();

// Get a value
QString ip = cfg->getValue("connection", "last_ip", "").toString();

// Set a value
cfg->setValue("connection", "last_ip", "192.168.88.1");

// Save to disk
cfg->save();
```

## Persistence Across Tab Switches
Settings are stored in memory and persist when switching between tabs. The configuration is only written to disk when:
1. Application closes
2. Explicit save is called

This ensures settings remain consistent across the entire session.

## Example config.json
```json
{
    "connection": {
        "last_ip": "192.168.88.1",
        "subnet": "192.168.88",
        "client_mode": false
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
    "panorama": {
        "rows": 3,
        "cols": 3
    },
    "media": {
        "download_dir": "/home/user/Downloads"
    }
}
```

## Benefits
1. **Human-readable**: JSON format is easy to edit manually if needed
2. **Portable**: Config file is in the app directory, easy to backup/share
3. **Persistent**: All settings survive app restarts
4. **Tab-independent**: Settings persist when switching between tabs
5. **No Qt dependencies**: Uses standard JSON, not QSettings internal format
