# Media Delete via MTP - User Guide

## Was ist passiert?

HTTP Delete wird vom DWARF II nicht unterstützt (`server replied ... not implemented`).

**Neue Lösung:** Automatischer Fallback auf MTP (USB-Verbindung)

## Wie funktioniert es jetzt?

### Automatischer Ablauf

1. **Versuch 1:** HTTP Delete über WiFi
   - Schnell, keine USB nötig
   - ❌ Schlägt fehl mit "not implemented"

2. **Versuch 2:** MTP Delete über USB (automatisch)
   - Erfordert USB-Verbindung zum DWARF II
   - ✅ Funktioniert zuverlässig
   - Platform-spezifische MTP-Tools

### User Experience

```
[Click Delete] → HTTP versucht → Fehlschlag
              ↓
              MTP automatisch aktiviert
              ↓
              "Connect via USB" Hinweis (falls nicht verbunden)
              ↓
              Löschen via MTP
              ↓
              Erfolg! Gallery aktualisiert
```

## Installation Requirements

### Windows ✅

**Keine Installation nötig!**

Windows hat MTP built-in. DWARF II einfach per USB verbinden.

```
1. USB Kabel anschließen
2. DWARF II wird als "Portable Device" erkannt
3. Delete funktioniert automatisch
```

### Linux 🐧

**MTP Tools installieren:**

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install mtp-tools

# Fedora/RHEL
sudo dnf install libmtp-examples

# Arch Linux
sudo pacman -S libmtp

# Test
mtp-detect
```

**Verbindung:**
```bash
1. USB Kabel anschließen
2. Device sollte erkannt werden: mtp-detect
3. Delete funktioniert automatisch
```

**Troubleshooting (Linux):**

Falls Device nicht erkannt:
```bash
# Check USB connection
lsusb | grep -i dwarf

# Check MTP service
systemctl status --user gvfs-mtp-volume-monitor

# Manual file listing
mtp-files

# Manual delete (test)
mtp-delfile -n <file-id>
```

**Permissions Fix:**
```bash
# Add udev rule for DWARF II
sudo nano /etc/udev/rules.d/51-dwarf-mtp.rules

# Add line (replace XXXX:YYYY with vendor:product from lsusb)
SUBSYSTEM=="usb", ATTR{idVendor}=="XXXX", ATTR{idProduct}=="YYYY", MODE="0666"

# Reload rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Reconnect device
```

### macOS 🍎

**MTP Tools via Homebrew:**

```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install libmtp
brew install libmtp

# Test
mtp-detect
```

**Alternative: Android File Transfer**
- Download: https://www.android.com/filetransfer/
- GUI app für MTP devices
- Delete muss aber über die App erfolgen

## In der Anwendung

### Normaler Workflow

1. **Gallery öffnen**
   - Tab "Gallery" in der App
   - Bilder werden über FTP/HTTP geladen

2. **Bild auswählen**
   - Click auf Bild → Lightbox öffnet
   - Delete Button klicken

3. **Bestätigung**
   - "Are you sure?" Dialog

4. **Automatischer Löschvorgang**
   ```
   [HTTP Versuch über WiFi]
   ↓ (schlägt fehl)
   [MTP Versuch über USB]
   ↓
   Erfolg oder Fehlermeldung
   ```

### Fehler-Szenarien

#### "HTTP delete not supported, trying USB/MTP..."
→ Normal! App versucht jetzt MTP.

#### "MTP tools not installed"
**Linux/macOS:**
```
Install MTP tools with:
  Linux: sudo apt-get install mtp-tools
  macOS: brew install libmtp
```

**Windows:** Sollte nicht passieren (built-in)

#### "DWARF II not connected via USB"
→ USB-Kabel anschließen und erneut versuchen.

#### "File not found on device"
→ Datei wurde möglicherweise schon gelöscht oder befindet sich in anderem Verzeichnis.

## Technical Details

### Platform-Specific Implementation

**Windows:**
- PowerShell mit Shell.Application
- Native WPD (Windows Portable Devices)
- Kein extra Tool nötig

**Linux:**
- `mtp-detect` - Device erkennen
- `mtp-files` - Dateiliste mit IDs
- `mtp-delfile -n <file-id>` - Datei löschen

**macOS:**
- libmtp (via Homebrew)
- Gleiche Tools wie Linux
- Native MTP Support fehlt in macOS

### File Path Resolution

MTP nutzt Object IDs statt Pfade:
```
1. Get file list: mtp-files
2. Parse file name → Object ID
3. Delete: mtp-delfile -n <object-id>
```

### Timeout

- MTP Operations: 10 Sekunden Timeout
- Bei langsamer USB-Verbindung kann es länger dauern
- Timeout-Error → Erneut versuchen

## Troubleshooting

### Windows

**Problem:** "DWARF II device not found"
**Lösung:**
1. USB Kabel prüfen
2. Device Manager öffnen → "Portable Devices"
3. DWARF II sollte dort erscheinen
4. Falls "Unknown Device" → Treiber neu installieren

**Problem:** "Access denied"
**Lösung:**
- Als Administrator ausführen (Rechtklick → "Run as Administrator")

### Linux

**Problem:** "No raw devices found"
**Lösung:**
```bash
# Check if device is connected
lsusb | grep -i dwarf

# Check MTP service
systemctl --user restart gvfs-mtp-volume-monitor

# Try manual detection
mtp-detect

# Check permissions
ls -la /dev/bus/usb/*/*  # Find device
sudo chmod 666 /dev/bus/usb/XXX/YYY  # Temporary fix
```

**Problem:** "Permission denied"
**Lösung:** Udev rule hinzufügen (siehe oben)

### macOS

**Problem:** "MTP tools not found"
**Lösung:**
```bash
brew install libmtp
# Oder Android File Transfer nutzen
```

**Problem:** "Device not recognized"
**Lösung:**
- Android File Transfer app installieren
- DWARF II verbinden
- App sollte Device erkennen

## Performance

**HTTP (WiFi):**
- ❌ Nicht unterstützt vom DWARF II
- ⚡ Wäre am schnellsten (wenn es ginge)

**MTP (USB):**
- ✅ Funktioniert zuverlässig
- 🐌 Etwas langsamer (USB Overhead)
- 🔌 Erfordert Kabelverbindung

**FTP:**
- ✅ Funktioniert für Download
- ❌ Delete nicht unterstützt (read-only)

## Future Improvements

- [ ] Batch delete (mehrere Dateien auf einmal)
- [ ] MTP Connection Status Indicator
- [ ] Automatic USB reconnection detection
- [ ] MTP file browser (nicht nur delete)
- [ ] Better error messages mit Troubleshooting links
- [ ] macOS: Native MTP statt CLI tools

## Support

Bei Problemen:
1. Check USB connection (lsusb / Device Manager)
2. Install MTP tools (Linux/macOS)
3. Try manual MTP test (mtp-detect, mtp-files)
4. Check app console output (debug logs)
5. GitHub Issues: https://github.com/jeamy/zwerg_ii/issues

## Summary

✅ **DELETE FUNKTIONIERT JETZT!**

**Workflow:**
1. Connect DWARF II via USB (zusätzlich zu WiFi)
2. Delete button in Gallery → Automatisch MTP
3. File wird gelöscht
4. Gallery aktualisiert

**Installation:**
- Windows: Nothing ✅
- Linux: `sudo apt-get install mtp-tools`
- macOS: `brew install libmtp`

**Alternative:**
- Offizielle DWARF App
- Windows Explorer (MTP Drag&Drop)
- Android File Transfer (macOS)
