# DWARF II App - Schritt-für-Schritt Implementierung

## Übersicht

**Ziel**: Web-App zur Steuerung des DWARF II Teleskops  
**Tech-Stack**: FastAPI (Backend), SQLite3 (DB), HTML/JS/CSS (Frontend)  
**Dauer**: 10-14 Tage

---

## Phase 1: Backend-Setup (Tag 1-2)

### Tag 1: Grundgerüst

1. **Projekt initialisieren**
   ```bash
   cd /media/data/programming/zwergii
   mkdir -p backend/app/{api,models,services,utils}
   mkdir -p backend/proto
   mkdir -p frontend/{css,js/components,js/utils}
   mkdir -p database
   ```

2. **Dependencies installieren**
   - FastAPI, Uvicorn
   - WebSockets, Protobuf
   - SQLAlchemy, aiosqlite
   - httpx

3. **Datenbank-Modelle erstellen**
   - Device, Session, Media

4. **FastAPI-App erstellen**
   - CORS konfigurieren
   - Router einbinden

### Tag 2: Protobuf & Clients

1. **Protobuf-Dateien kompilieren**
   - astro.proto, camera.proto, focus.proto, etc.

2. **HTTP-Client implementieren**
   - DwarfHTTPClient mit allen Endpoints

3. **WebSocket-Client implementieren**
   - DwarfWebSocketClient mit Protobuf

4. **Konstanten definieren**
   - Alle CMD-Codes, Module-IDs

---

## Phase 2: API-Endpoints (Tag 3-5)

### Tag 3: Device & Camera

1. **Device-API**
   - POST /api/device/connect
   - GET /api/device/info
   - GET /api/device/firmware

2. **Camera-API**
   - POST /api/camera/open
   - POST /api/camera/close
   - POST /api/camera/photo
   - GET /api/camera/stream/{type}

### Tag 4: Astro & Focus

1. **Astro-API**
   - POST /api/astro/calibration/start
   - POST /api/astro/goto/dso
   - POST /api/astro/goto/solar
   - POST /api/astro/stacking/start

2. **Focus-API**
   - POST /api/focus/auto
   - POST /api/focus/astro/start
   - POST /api/focus/manual/step

### Tag 5: Motor & System

1. **Motor-API**
   - POST /api/motor/run
   - POST /api/motor/stop
   - POST /api/motor/joystick

2. **System-API**
   - POST /api/system/time
   - POST /api/system/shutdown
   - POST /api/system/reboot

---

## Phase 3: Frontend (Tag 6-8)

### Tag 6: Basis-UI

1. **HTML-Struktur**
   - Header, Navigation, Main-Content

2. **CSS-Styling**
   - Responsive Design
   - Dark Theme

3. **API-Client (JS)**
   - constants.js
   - api-client.js

### Tag 7: Komponenten

1. **Connection-View**
   - IP-Eingabe
   - Verbindungs-Status

2. **Camera-Control**
   - Kamera öffnen/schließen
   - Foto/Video aufnehmen
   - Live-Stream anzeigen
   - Parameter-Steuerung

3. **Astro-Control**
   - Kalibrierung
   - GOTO-Steuerung
   - Stacking-Steuerung

### Tag 8: Erweiterte Features

1. **Focus-Control**
   - Auto/Manual-Fokus
   - Astro-Fokus

2. **Motor-Control**
   - Joystick-Steuerung
   - Direkte Motor-Steuerung

3. **Album-View**
   - Medien-Liste
   - Vorschau
   - Download

---

## Phase 4: Integration & Testing (Tag 9-10)

### Tag 9: WebSocket-Integration

1. **WebSocket-Handler (Backend)**
   - Bidirektionale Kommunikation
   - Benachrichtigungen

2. **WebSocket-Client (Frontend)**
   - Verbindung aufbauen
   - Events empfangen
   - Status-Updates

### Tag 10: Testing & Debugging

1. **Backend-Tests**
   - API-Endpoints testen
   - WebSocket-Verbindung testen

2. **Frontend-Tests**
   - UI-Funktionen testen
   - Stream-Anzeige testen

3. **End-to-End-Tests**
   - Komplette Workflows testen

---

## Wichtige Dateien

### Backend
- `backend/app/main.py` - FastAPI App
- `backend/app/services/dwarf_client.py` - HTTP-Client
- `backend/app/services/dwarf_ws.py` - WebSocket-Client
- `backend/app/utils/constants.py` - API-Konstanten

### Frontend
- `frontend/index.html` - Haupt-HTML
- `frontend/js/app.js` - Haupt-App
- `frontend/js/constants.js` - API-Endpunkte
- `frontend/js/api-client.js` - HTTP-Client

---

## Nächste Schritte

1. Backend-Grundgerüst erstellen
2. Protobuf-Dateien kompilieren
3. HTTP- und WebSocket-Clients implementieren
4. API-Endpoints entwickeln
5. Frontend-UI aufbauen
6. Komponenten implementieren
7. Integration testen
8. Dokumentation vervollständigen
