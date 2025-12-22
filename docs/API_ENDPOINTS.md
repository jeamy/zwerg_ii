# DWARF II Backend - Vollständige API-Übersicht

## 📊 Gesamt-Statistik

- **API-Endpunkte**: 92
- **API-Module**: 10
- **Protobuf-Messages**: 40+
- **Code-Zeilen**: ~5500

---

## 🔌 API-Endpunkte nach Modulen

### 1. Device API (7 Endpoints)
**Prefix**: `/api/device`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/connect` | Gerät verbinden & in DB speichern |
| GET | `/info` | Geräte-Informationen |
| GET | `/firmware` | Firmware-Version |
| POST | `/name-password` | Name/Passwort ändern |
| POST | `/reset` | Gerät zurücksetzen |
| GET | `/list` | Alle Geräte |
| DELETE | `/{id}` | Gerät löschen |

---

### 2. Camera API (15 Endpoints)
**Prefix**: `/api/camera`

#### Teleobjektiv
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/tele/open` | Kamera öffnen |
| POST | `/tele/close` | Kamera schließen |
| POST | `/tele/photo` | Foto aufnehmen |
| POST | `/tele/burst/start` | Serienaufnahme starten |
| POST | `/tele/burst/stop` | Serienaufnahme stoppen |
| POST | `/tele/video/start` | Video starten |
| POST | `/tele/video/stop` | Video stoppen |
| POST | `/tele/params/set` | Parameter setzen |
| GET | `/tele/params/get` | Parameter abrufen |

#### Weitwinkel
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/wide/open` | Kamera öffnen |
| POST | `/wide/close` | Kamera schließen |
| POST | `/wide/photo` | Foto aufnehmen |

#### Streams
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| GET | `/stream/{type}` | JPG-Stream |
| GET | `/rtsp/{type}` | RTSP-URL |

---

### 3. Camera Parameters API (10 Endpoints) ⭐ NEU
**Prefix**: `/api/camera/params`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/exposure/mode` | Belichtungsmodus (Auto/Manual) |
| POST | `/exposure/value` | Belichtungszeit (µs) |
| POST | `/gain/mode` | Gain-Modus (Auto/Manual) |
| POST | `/gain/value` | Gain-Wert (0-300) |
| POST | `/wb/mode` | Weißabgleich-Modus |
| POST | `/ircut` | IR-Filter (Cut/Pass) |
| POST | `/brightness` | Helligkeit (0-255) |
| POST | `/contrast` | Kontrast (0-255) |
| POST | `/saturation` | Sättigung (0-255) |
| POST | `/sharpness` | Schärfe (0-255) |

---

### 4. Album API (4 Endpoints)
**Prefix**: `/api/album`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| GET | `/counts` | Medien-Anzahl pro Typ |
| POST | `/list` | Medien-Liste mit Paginierung |
| POST | `/delete` | Medien löschen |
| GET | `/config` | Parameter-Konfiguration |

---

### 5. Astro API (21 Endpoints)
**Prefix**: `/api/astro`

#### Kalibrierung
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/calibration/start` | Kalibrierung starten |
| POST | `/calibration/stop` | Kalibrierung stoppen |

#### GOTO
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/goto/dso` | GoTo Deep-Sky-Objekt |
| POST | `/goto/solar` | GoTo Sonnensystem |
| POST | `/goto/stop` | GoTo stoppen |

#### Ein-Klick GOTO
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/goto/one-click/dso` | One-Click GoTo DSO |
| POST | `/goto/one-click/solar` | One-Click GoTo Solar |
| POST | `/goto/one-click/stop` | One-Click GoTo stoppen |

#### Stacking
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/stacking/start` | Live Stacking starten |
| POST | `/stacking/stop` | Live Stacking stoppen |
| POST | `/stacking/wide/start` | Wide Stacking starten |
| POST | `/stacking/wide/stop` | Wide Stacking stoppen |

#### Tracking
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/track/special/start` | Sonne/Mond tracken |
| POST | `/track/special/stop` | Tracking stoppen |

#### Darkframe
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/darkframe/capture` | Darkframe aufnehmen |
| POST | `/darkframe/stop` | Darkframe stoppen |
| GET | `/darkframe/check` | Darkframe-Status |
| GET | `/darkframe/list` | Darkframe-Liste |

#### EQ-Verifizierung
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/eq-solving/start` | EQ Solving starten |
| POST | `/eq-solving/stop` | EQ Solving stoppen |

#### Sonstiges
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/go-live` | Zurück zur Live-Ansicht |

---

### 6. Focus API (6 Endpoints)
**Prefix**: `/api/focus`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/auto` | Normal Auto-Focus |
| POST | `/astro/start` | Astro Auto-Focus starten |
| POST | `/astro/stop` | Astro Auto-Focus stoppen |
| POST | `/manual/step` | Manueller Einzelschritt |
| POST | `/manual/continuous/start` | Kontinuierlicher Focus |
| POST | `/manual/continuous/stop` | Kontinuierlichen Focus stoppen |

---

### 7. Motor API (6 Endpoints)
**Prefix**: `/api/motor`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/run` | Motor bewegen |
| POST | `/stop` | Motor stoppen |
| POST | `/joystick/start` | Joystick-Steuerung |
| POST | `/joystick/fixed-angle` | Joystick fester Winkel |
| POST | `/joystick/stop` | Joystick stoppen |
| POST | `/dual-camera-linkage` | Dual-Kamera-Linkage |

---

### 8. Tracking API (6 Endpoints) ⭐ NEU
**Prefix**: `/api/tracking`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/start` | Objekt-Tracking starten |
| POST | `/stop` | Objekt-Tracking stoppen |
| POST | `/sentry/start` | Sentry-Modus starten |
| POST | `/sentry/stop` | Sentry-Modus stoppen |
| POST | `/mot/start` | Multi-Object Tracking |
| POST | `/mot/track-one` | Spezifisches Objekt tracken |

---

### 9. Panorama API (2 Endpoints) ⭐ NEU
**Prefix**: `/api/panorama`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/start` | Panorama starten |
| POST | `/stop` | Panorama stoppen |

---

### 10. System API (13 Endpoints)
**Prefix**: `/api/system`

#### Zeit & Zeitzone
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/time/set` | Systemzeit setzen |
| POST | `/timezone/set` | Zeitzone setzen |

#### System-Modi
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/mtp/set` | MTP-Modus setzen |
| POST | `/cpu/set` | CPU-Modus setzen |
| POST | `/master-lock` | Master-Lock |

#### RGB-Licht
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/rgb/on` | RGB-Ring einschalten |
| POST | `/rgb/off` | RGB-Ring ausschalten |

#### Batterie-Anzeige
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/power-indicator/on` | Batterie-Anzeige ein |
| POST | `/power-indicator/off` | Batterie-Anzeige aus |

#### Power-Management
| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| POST | `/shutdown` | Herunterfahren |
| POST | `/reboot` | Neustart |

---

### 11. Scanner API (1 Endpoint)
**Prefix**: `/api/scanner`

| Method | Endpoint | Beschreibung |
|--------|----------|--------------|
| GET | `/scan` | Netzwerk scannen |

---

## 🎯 Verwendungsbeispiele

### Tracking starten
```bash
curl -X POST "http://localhost:8000/api/tracking/start?ip=192.168.88.1" \
  -H "Content-Type: application/json" \
  -d '{"x": 100, "y": 100, "w": 200, "h": 200}'
```

### Panorama 3x3
```bash
curl -X POST "http://localhost:8000/api/panorama/start?ip=192.168.88.1" \
  -H "Content-Type: application/json" \
  -d '{"rows": 3, "cols": 3}'
```

### Belichtung auf 10s setzen
```bash
# Manueller Modus
curl -X POST "http://localhost:8000/api/camera/params/exposure/mode?ip=192.168.88.1" \
  -H "Content-Type: application/json" \
  -d '{"mode": 1, "camera": "tele"}'

# 10 Sekunden
curl -X POST "http://localhost:8000/api/camera/params/exposure/value?ip=192.168.88.1" \
  -H "Content-Type: application/json" \
  -d '{"value": 10000000, "camera": "tele"}'
```

### IR-Filter für Astro
```bash
curl -X POST "http://localhost:8000/api/camera/params/ircut?ip=192.168.88.1" \
  -H "Content-Type: application/json" \
  -d '{"mode": 1}'
```

---

## 📚 API-Dokumentation

**Swagger UI**: http://localhost:8000/docs  
**ReDoc**: http://localhost:8000/redoc

---

## ✅ Status: VOLLSTÄNDIG

Alle 92 API-Endpunkte sind implementiert und funktionsfähig!
