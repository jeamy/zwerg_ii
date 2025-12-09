# DWARF II Teleskop-Steuerungs-App

Web-basierte Anwendung zur vollständigen Steuerung des DWARF II Teleskops.

## Features

- 📷 **Kamera-Steuerung**: Tele- und Weitwinkel-Kamera
- 🌌 **Astronomie-Funktionen**: GOTO, Tracking, Stacking
- 🎯 **Fokussierung**: Auto- und Manualfokus, Astro-Fokus
- 🕹️ **Motor-Steuerung**: Joystick und direkte Steuerung
- 📁 **Album-Verwaltung**: Medien durchsuchen und verwalten
- 🔴 **Live-Stream**: Echtzeit-Bildübertragung

## Tech-Stack

- **Backend**: FastAPI (Python)
- **Datenbank**: SQLite3
- **Frontend**: HTML, JavaScript (Vanilla), CSS
- **Kommunikation**: HTTP REST API, WebSocket, Protocol Buffers

## Projekt-Struktur

```
zwergii/
├── backend/              # FastAPI Backend
│   ├── app/
│   │   ├── api/         # API-Endpoints
│   │   ├── models/      # Datenbank-Modelle
│   │   ├── services/    # DWARF II Clients
│   │   └── utils/       # Hilfsfunktionen
│   ├── proto/           # Protobuf-Definitionen
│   └── requirements.txt
├── frontend/            # Web-Frontend
│   ├── css/            # Stylesheets
│   ├── js/             # JavaScript
│   └── index.html
├── database/           # SQLite-Datenbank
└── doc/               # Dokumentation
```

## Installation

### Voraussetzungen

- Python 3.10+
- pip
- protoc (Protocol Buffers Compiler)

### Backend einrichten

```bash
# Virtual Environment erstellen
python -m venv venv
source venv/bin/activate  # Linux/Mac
# oder
venv\Scripts\activate     # Windows

# Dependencies installieren
cd backend
pip install -r requirements.txt

# Protobuf-Dateien kompilieren
cd proto
protoc --python_out=../app/services/proto/ *.proto
```

### Frontend einrichten

Keine Installation nötig - einfach `frontend/index.html` im Browser öffnen oder über einen Webserver bereitstellen.

## Verwendung

### Backend starten

```bash
cd backend
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

API-Dokumentation: http://localhost:8000/docs

### Frontend öffnen

1. **Über Backend**: http://localhost:8000/static/ im Browser öffnen (liefert das Frontend aus)
2. **Direkt**: `frontend/index.html` lokal öffnen oder eigenen Webserver nutzen:
   ```bash
   cd frontend
   python -m http.server 8080
   ```
   Dann http://localhost:8080 öffnen

### DWARF II verbinden

1. DWARF II einschalten
2. Mit DWARF II WLAN verbinden (AP-Modus) oder DWARF II mit eigenem WLAN verbinden (STA-Modus)
3. In der App IP-Adresse eingeben (Standard: `192.168.88.1`)
4. "Verbinden" klicken

## API-Dokumentation

Siehe `doc/DWARF_II_API_COMPLETE.md` für die vollständige DWARF II API-Dokumentation.

## Implementierungsanleitung

Siehe `doc/IMPLEMENTATION_STEPS.md` für die Schritt-für-Schritt Anleitung zur Implementierung.

## Entwicklung

### Backend-Entwicklung

```bash
# Tests ausführen
pytest

# Code-Formatierung
black app/
isort app/

# Linting
flake8 app/
```

### Frontend-Entwicklung

- Keine Build-Tools erforderlich
- Vanilla JavaScript (ES6+)
- CSS ohne Preprocessor
- Externe CSS/JS in separaten Dateien (siehe Memory)

## API-Endpoints

### Device
- `POST /api/device/connect` - Gerät verbinden
- `GET /api/device/info` - Geräte-Info abrufen
- `GET /api/device/firmware` - Firmware-Version

### Camera
- `POST /api/camera/open` - Kamera öffnen
- `POST /api/camera/close` - Kamera schließen
- `POST /api/camera/photo` - Foto aufnehmen
- `POST /api/camera/video/start` - Video starten
- `POST /api/camera/video/stop` - Video stoppen
- `GET /api/camera/stream/{type}` - Live-Stream

### Astro
- `POST /api/astro/calibration/start` - Kalibrierung starten
- `POST /api/astro/goto/dso` - GOTO Deep-Sky-Objekt
- `POST /api/astro/goto/solar` - GOTO Sonnensystem
- `POST /api/astro/stacking/start` - Stacking starten
- `POST /api/astro/stacking/stop` - Stacking stoppen

### Focus
- `POST /api/focus/auto` - Autofokus
- `POST /api/focus/astro/start` - Astro-Autofokus
- `POST /api/focus/manual/step` - Manueller Schritt

### Motor
- `POST /api/motor/run` - Motor bewegen
- `POST /api/motor/stop` - Motor stoppen
- `POST /api/motor/joystick` - Joystick-Steuerung

## Konfiguration

### Backend-Konfiguration

`backend/app/config.py`:
```python
DATABASE_URL = "sqlite+aiosqlite:///./database/dwarf.db"
DWARF_DEFAULT_IP = "192.168.88.1"
DWARF_HTTP_PORT = 8082
DWARF_WS_PORT = 9900
```

### Frontend-Konfiguration

`frontend/js/constants.js`:
```javascript
export const API_BASE_URL = 'http://localhost:8000/api';
export const WS_URL = 'ws://localhost:8000/ws';
```

## Fehlerbehebung

### Verbindung zum DWARF II fehlgeschlagen

- Überprüfen Sie die IP-Adresse
- Stellen Sie sicher, dass Sie mit dem DWARF II WLAN verbunden sind
- Prüfen Sie, ob Port 8082 und 9900 erreichbar sind

### WebSocket-Verbindung bricht ab

- Heartbeat wird alle 30 Sekunden gesendet
- Überprüfen Sie die Netzwerkstabilität
- Prüfen Sie Firewall-Einstellungen

### Stream wird nicht angezeigt

- Überprüfen Sie, ob die Kamera geöffnet ist
- Prüfen Sie die Stream-URL
- Browser-Konsole auf Fehler überprüfen

## Lizenz

MIT License

## Kontakt

Bei Fragen oder Problemen öffnen Sie bitte ein Issue.

## Roadmap

- [ ] Phase 1: Backend-Grundgerüst ✅
- [ ] Phase 2: API-Endpoints
- [ ] Phase 3: Frontend-UI
- [ ] Phase 4: WebSocket-Integration
- [ ] Phase 5: Testing & Debugging
- [ ] Phase 6: Deployment
# zwergii
