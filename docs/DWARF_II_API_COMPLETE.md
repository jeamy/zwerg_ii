# DWARF II Teleskop - Vollständige API-Dokumentation

Version: 2.0 | Stand: November 2024

## 1. Netzwerk & Verbindung

### IP-Adressen
- **AP-Modus**: `192.168.88.1` (fest)
- **STA-Modus**: Über Bluetooth abfragen

### Ports
- **HTTP Standard**: 8082
- **HTTP JPG**: 8092
- **WebSocket**: 9900
- **RTSP Tele**: `rtsp://192.168.88.1/ch0/stream0`
- **RTSP Wide**: `rtsp://192.168.88.1/ch1/stream0`

### Standard-Zugangsdaten
- **Gerätename**: `DWARF_XXXXXX`
- **Passwort**: `DWARF_12345678`
- **BLE Service ID**: `0000DAF2-0000-1000-8000-00805F9B34FB`

---

## 2. HTTP API Endpoints

### 2.1 Geräte-Management

#### GET /deviceInfo
Geräte-Informationen abrufen

#### POST /setDeviceNameAndPsd
Gerätename oder Passwort ändern
```json
{
  "mode": 1,
  "oldValue": "old",
  "newValue": "new"
}
```

#### POST /firmwareVersion
Firmware-Version abrufen

#### POST /uploadFirmware
Firmware hochladen (multipart/form-data)

### 2.2 Album-Verwaltung

#### POST /album/list/mediaCounts
Anzahl Medien pro Typ

#### POST /album/list/mediaInfos
Medien-Liste mit Paginierung

#### POST /album/delete
Dateien löschen

### 2.3 Bild-Streams

#### GET http://IP:8092/mainstream
Teleobjektiv JPG-Stream

#### GET http://IP:8092/secondstream
Weitwinkel JPG-Stream

---

## 3. WebSocket API

### 3.1 Verbindung
```
ws://IP:9900/
```

### 3.2 Protobuf-Struktur
```protobuf
message WsPacket {
  uint32 major_version = 1;
  uint32 minor_version = 2;
  uint32 device_id = 3;
  uint32 module_id = 4;
  uint32 cmd = 5;
  uint32 type = 6;
  bytes data = 7;
  string client_id = 8;
}
```

### 3.3 Module

| Modul | ID | Befehle | Beschreibung |
|-------|----|---------| -------------|
| CAMERA_TELE | 1 | 10000-10499 | Teleobjektiv |
| CAMERA_WIDE | 2 | 12000-12499 | Weitwinkel |
| ASTRO | 3 | 11000-11499 | Astronomie |
| SYSTEM | 4 | 13000-13299 | System |
| RGB_POWER | 5 | 13500-13799 | RGB & Power |
| MOTOR | 6 | 14000-14499 | Motor |
| TRACK | 7 | 14800-14899 | Tracking |
| FOCUS | 8 | 15000-15099 | Fokus |
| NOTIFY | 9 | 15200-15499 | Benachrichtigungen |

---

## 4. Wichtigste Befehle

### 4.1 Kamera (Teleobjektiv)
- **10000**: Kamera öffnen
- **10001**: Kamera schließen
- **10002**: Foto aufnehmen
- **10003**: Serienaufnahme starten
- **10005**: Video starten
- **10006**: Video stoppen
- **10035**: Alle Parameter setzen
- **10036**: Alle Parameter abrufen

### 4.2 Fokus
- **15000**: Normal-Autofokus
- **15004**: Astro-Autofokus starten
- **15005**: Astro-Autofokus stoppen
- **15001**: Manueller Einzelschritt

### 4.3 Astronomie
- **11000**: Kalibrierung starten
- **11002**: GOTO Deep-Sky
- **11003**: GOTO Sonnensystem
- **11005**: Stacking starten
- **11006**: Stacking stoppen
- **11013**: Ein-Klick GOTO DSO
- **11018**: EQ-Verifizierung

### 4.4 Motor
- **14000**: Motor bewegen
- **14002**: Motor stoppen
- **14006**: Joystick-Steuerung

### 4.5 System
- **13000**: Zeit setzen
- **13001**: Zeitzone setzen
- **13502**: Herunterfahren
- **13505**: Neustart

---

## 5. Fehlercodes

### HTTP
- **0**: Erfolg
- **-1**: Datei nicht gefunden
- **-2**: Ungültiger Parameter
- **-3**: MD5-Fehler

### WebSocket
- **0**: Erfolg
- **-1**: Protobuf-Fehler
- **-2**: SD-Karte nicht gefunden
- **-3**: Ungültiger Parameter
- **-4**: SD-Schreibfehler
- **-5**: Gerät nicht aktiviert
- **-6**: Speicher voll

### Kamera
- **10501**: Kamera aus
- **10502**: ISP-Fehler
- **10504**: Öffnen fehlgeschlagen

### Astronomie
- **11500**: Plate-Solving fehlgeschlagen
- **11501**: Funktion beschäftigt
- **11502**: Gain außerhalb Bereich
- **11503**: Darkframe nicht gefunden
- **11505**: GOTO fehlgeschlagen

---

## 6. Wichtige Datenstrukturen

### Sonnensystem-Ziele
1. Merkur
2. Venus
3. Mars
4. Jupiter
5. Saturn
6. Uranus
7. Neptun
8. Mond
9. Sonne

### Medien-Typen
0. Alle
1. Normale Fotos
2. Videos
3. Serienaufnahmen
4. Astronomie
5. Panorama

### Ein-Klick GOTO Schritte
- **10**: Himmelserkennung
- **20**: Fokussierung
- **30**: Kalibrierung
- **40**: GOTO

---

## 7. Parameter-Bereiche

### Helligkeit/Kontrast/Sättigung
- UI: -100 bis 100
- Intern: 0 bis 255
- Formel: `(UI + 100) * 255 / 200`

### Farbton
- UI: -180 bis 180
- Intern: 0 bis 255
- Formel: `(UI + 180) * 255 / 360`

### Schärfe
- UI: 0 bis 100
- Intern: 0 bis 100
- Formel: `UI = Intern`
