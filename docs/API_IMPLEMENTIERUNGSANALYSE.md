# zwergII: Analyse der DWARF-II-API-Implementierung

Stand: 2026-03-11

Quellenbasis:
- Offizielle/halb-offizielle Spezifikation aus [docs/dwarf_api_extracted.txt](/media/data/programming/zwergII/docs/dwarf_api_extracted.txt) und [docs/DWARF API2.0 - Feishu Docs.pdf](/media/data/programming/zwergII/docs/DWARF%20API2.0%20-%20Feishu%20Docs.pdf)
- Aktuelle Qt/C++-Implementierung in `src/`

## Kurzfazit

`zwergII` implementiert den praxisrelevanten Kern der DWARF-II-Steuerung bereits recht breit:
- WebSocket-Kommunikation
- Tele/Wide-Kamera mit Live-MJPEG, Foto, Tele-Video und großen Teilen der Parametersteuerung
- Astro-Kernfunktionen wie `Go Live`, Kalibrierung, One-Click-GOTO, Stacking und Sun/Moon-Tracking
- Motorsteuerung und Basis-Fokus
- Panorama
- Medienliste, Thumbnails und Download

Nicht vollständig umgesetzt ist die API aber klar in den Bereichen:
- Device-/HTTP-Management
- Tracking-Modul 7
- vollständige Fokus-API
- dokumentierte Album-Delete-API
- dokumentierte RTSP-Nutzung

Zusätzlich gibt es mehrere Stellen, an denen `zwergII` nicht der dokumentierten API folgt, sondern auf Reverse Engineering und Firmware-Workarounds setzt. Das ist funktional nachvollziehbar, aber nicht gleichbedeutend mit sauberer Spezifikationsabdeckung.

## Bewertungslogik

- `Implementiert`: im aktuellen Code vorhanden und in die Laufzeit/UI eingebunden
- `Teilweise`: nur Subset umgesetzt, nur Controller-Level vorhanden oder stark heuristisch
- `Fehlend`: in der Spezifikation vorhanden, im aktuellen Qt-Code aber nicht umgesetzt
- `Abweichend/Falsch`: Implementierung nutzt andere Endpunkte/Befehle oder fragwürdige Annahmen als die Spezifikation

## Modulübersicht

| API-Bereich | Status | Einschätzung |
|---|---|---|
| HTTP Device API | Teilweise | Discovery vorhanden, aber mit abweichendem Endpoint; Name/Passwort/Firmware-Upload fehlen |
| HTTP Album API | Teilweise | `mediaInfos` vorhanden; `mediaCounts` fehlt; Delete weicht von Spec ab |
| JPG Stream | Implementiert | MJPEG `mainstream`/`secondstream` wird genutzt |
| RTSP Stream | Fehlend | In Doku vorhanden, im Qt-Client nicht verwendet |
| Kamera Tele | Teilweise bis weitgehend | Open/Close/Photo/Video/Params da; Burst/Timelapse fehlen |
| Kamera Wide | Teilweise | Open/Close/Photo/Params da; Burst/Timelapse fehlen |
| Astro | Teilweise bis weitgehend | Kernfunktionen da; nicht alle dokumentierten Kommandos sind UI-seitig fertig |
| Fokus | Teilweise | Nur Normal-AF und Single-Step manuell |
| Motor | Teilweise | Run/Stop/Joystick da; Dual-Camera-Linkage fehlt |
| Tracking (Modul 7) | Fehlend | Proto vorhanden, aber keine Controller-/UI-Umsetzung |
| Panorama | Teilweise | Läuft, aber reverse-engineered und fragil |
| System (Modul 4) | Weitgehend implementiert | Zeit, Zeitzone, MTP und CPU sind als Controller + UI verdrahtet |
| RGB/Power (Modul 5) | Implementiert | RGB-Ring, Power-Indikator, Shutdown und Reboot sind umgesetzt |

## Detaillierte Analyse

### 1. Transport / Dispatcher

Implementiert:
- WebSocket-Client mit Protobuf-WsPacket und Heartbeat ist vorhanden in [src/net/DwarfWebSocketClient.cpp](/media/data/programming/zwergII/src/net/DwarfWebSocketClient.cpp#L137).
- Dispatcher kennt alle relevanten Module 1-15 in [src/net/DwarfMessageDispatcher.h](/media/data/programming/zwergII/src/net/DwarfMessageDispatcher.h#L17).

Teilweise:
- Im `MainWindow` werden effektiv nur Kamera, Astro und Panorama verdrahtet; `systemMessage`, `rgbPowerMessage`, `trackMessage` und `focusMessage` bleiben ungenutzt, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L324).

Bewertung:
- Technische Basis ist gut.
- Abdeckung auf Modulebene ist deutlich breiter als die tatsächlich benutzte Laufzeitverdrahtung.

### 2. HTTP Device API

Spezifikation:
- `GET/POST /deviceInfo`
- `POST /setDeviceNameAndPsd`
- `POST /firmwareVersion`
- `POST /uploadFirmware`

Implementiert:
- Discovery ruft Geräteinfo ab und extrahiert Name/Firmware heuristisch in [src/net/DwarfFinder.cpp](/media/data/programming/zwergII/src/net/DwarfFinder.cpp#L199).

Fehlt:
- Name/Passwort ändern
- dedizierter Firmware-Request
- Firmware-Upload

Abweichend/Falsch:
- Es wird `http://IP:8082/getdeviceInfo` verwendet statt des in der Doku beschriebenen `/deviceInfo`, siehe [src/net/DwarfFinder.cpp](/media/data/programming/zwergII/src/net/DwarfFinder.cpp#L199).
- Firmware wird nicht über den dokumentierten Endpoint abgefragt, sondern aus einer Device-Info-Antwort herausgeraten, siehe [src/net/DwarfFinder.cpp](/media/data/programming/zwergII/src/net/DwarfFinder.cpp#L265).

Bewertung:
- Discovery ist vorhanden, aber die Device-HTTP-API ist nur rudimentär implementiert.

### 3. HTTP Album API

Spezifikation:
- `/album/list/mediaCounts`
- `/album/list/mediaInfos`
- `/album/delete`

Implementiert:
- Medienliste via `/album/list/mediaInfos` in [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp#L15).
- Gallery/UI-Klassifikation nach `mediaType` in [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L2140).

Teilweise:
- `mediaCounts` wird nicht verwendet; die UI baut ihre Kategorien aus `mediaInfos` nach.

Fehlend:
- dokumentierter Delete-Endpoint `/album/delete`

Abweichend/Falsch:
- Delete nutzt stattdessen `/sdcard/deleteFile`, siehe [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp#L89).
- Wenn das nicht funktioniert, fällt die App auf USB/MTP zurück, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L2605) und [src/net/DwarfMtpClient.cpp](/media/data/programming/zwergII/src/net/DwarfMtpClient.cpp#L135).

Bewertung:
- Listen/Thumbnails/Download sind brauchbar.
- Delete ist keine saubere Spezifikationsumsetzung, sondern ein Workaround.

### 4. JPG- und RTSP-Streams

Spezifikation:
- JPG: `/mainstream`, `/secondstream`
- RTSP: `/ch0/stream0`, `/ch1/stream0`

Implementiert:
- MJPEG/JPG-Streams werden aktiv verwendet, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L705) und [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L3010).

Fehlend:
- RTSP wird im aktuellen Qt-Client nicht verwendet.

Bewertung:
- Livebild ist implementiert, aber nur über MJPEG, nicht über die dokumentierte RTSP-Variante.

### 5. Kamera-API

Implementiert:
- Open/Close/Photo für Tele und Wide, Video Start/Stop für Tele in [src/net/DwarfCameraController.h](/media/data/programming/zwergII/src/net/DwarfCameraController.h#L20).
- Exposure/Gain/WB/IR-Cut sowie Bildparameter in [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L313).
- `GET_ALL_PARAMS` und Response-Verarbeitung in [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L582).

Fehlend:
- Tele Burst `10003/10004`
- Tele Timelapse `10033/10034`
- Wide Burst `12023/12024`
- Wide Timelapse `12025/12026`
- dedizierte Nutzung von `GET_*`-Einzelparametern
- `GET_SYSTEM_WORKING_STATE` Tele `10039`

Teilweise:
- Parametersteuerung ist breit, aber nicht vollständig dokumentenkonform umgesetzt.

Abweichend/Falsch:
- Für Brightness/Contrast/Hue/Saturation/Sharpness werden nicht die dokumentierten Kamera-Setter verwendet, sondern reverse-engineerte Feature-Parameter über Modul 15 / Cmd 16703, siehe [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L402) und [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L881).
- Video-Start arbeitet mit Firmware-Heuristik und Reopen-Delay statt reinem Spec-Flow, siehe [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L253).

Bewertung:
- Für normale Nutzung ist die Kameraanbindung stark.
- Gegen die volle API fehlt aber ein relevanter Teil der Capture-Kommandos.

### 6. Fokus-API

Spezifikation:
- `15000` Normal-AF
- `15001` Single-Step
- `15002/15003` Continuous Focus
- `15004/15005` Astro AF Start/Stop

Implementiert:
- Nur `autoFocusNormal`, `manualStepNear`, `manualStepFar` in [src/net/DwarfFocusController.h](/media/data/programming/zwergII/src/net/DwarfFocusController.h#L12) und [src/net/DwarfFocusController.cpp](/media/data/programming/zwergII/src/net/DwarfFocusController.cpp#L18).
- UI nutzt genau nur diese drei Funktionen, siehe [src/ui/MotorControlPanel.cpp](/media/data/programming/zwergII/src/ui/MotorControlPanel.cpp#L244).

Fehlend:
- Continuous manual focus
- Astro autofocus
- Response-/Statushandling des Fokusmoduls

Bewertung:
- Basis-Fokus vorhanden.
- Gegenüber der dokumentierten Fokus-API klar nur ein Subset.

### 7. Astro-API

Implementiert:
- Controller deckt große Teile des Astro-Moduls ab: Kalibrierung, GOTO/One-Click-GOTO, `Go Live`, Stacking, Wide-Stacking, Darkframes, EQ-Solving, Special-Target-Tracking, siehe [src/net/DwarfAstroController.h](/media/data/programming/zwergII/src/net/DwarfAstroController.h#L18) und [src/net/DwarfAstroController.cpp](/media/data/programming/zwergII/src/net/DwarfAstroController.cpp#L18).
- Notifications für Akku, SD, Stacking, Kalibrierung, GOTO, Temperatur werden verarbeitet in [src/net/DwarfAstroController.cpp](/media/data/programming/zwergII/src/net/DwarfAstroController.cpp#L518).
- UI nutzt GoLive + Kalibrierung + One-Click-GOTO + Live-Stacking aktiv, siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1345) und [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1462).

Teilweise:
- Wide-Stacking, Darkframe-Management und EQ-Solving sind controllerseitig vorhanden, aber im sichtbaren UI nicht fertig integriert.
- `DEL_DARK_FRAME_LIST` ist im Controller gar nicht umgesetzt, obwohl der Cmd-Block definiert ist, siehe [src/net/DwarfAstroController.cpp](/media/data/programming/zwergII/src/net/DwarfAstroController.cpp#L40).

Abweichend:
- Mehrere Abläufe arbeiten mit zusätzlichen Delays und heuristischen Vorbedingungen (`Go Live`, Kameras öffnen, Kalibrierung abwarten), siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1349) und [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1515).

Bewertung:
- Astro ist eines der stärksten Module im Projekt.
- Vollständige API-Abdeckung ist aber noch nicht erreicht.

### 8. Motor-API

Spezifikation:
- `14000` Run
- `14002` Stop
- `14006` Joystick
- `14007` Fixed-Angle
- `14008` Stop Joystick
- `14009` Dual-Camera-Linkage

Implementiert:
- Run/Stop/Joystick/Fixed-Angle/Stop in [src/net/DwarfMotorController.cpp](/media/data/programming/zwergII/src/net/DwarfMotorController.cpp#L25).
- UI nutzt Joystick praktisch als Hauptsteuerung, siehe [src/ui/MotorControlPanel.cpp](/media/data/programming/zwergII/src/ui/MotorControlPanel.cpp#L259).

Fehlend:
- Dual-Camera-Linkage `14009`
- Response-/Statushandling des Motor-Moduls

Bewertung:
- Bewegungssteuerung ist nutzbar.
- API-seitig fehlt der letzte dokumentierte Funktionsblock.

### 9. Tracking-Modul 7

Spezifikation:
- `14800` Start Track
- `14801` Stop Track
- `14802/14803` Sentry
- `14804/14805` MOT
- weitere dokumentierte Tracking-Kommandos bis `14810`

Implementiert:
- Nur Proto-Definitionen vorhanden, siehe [src/proto/tracking.proto](/media/data/programming/zwergII/src/proto/tracking.proto#L1).
- Dispatcher kennt Modul 7, siehe [src/net/DwarfMessageDispatcher.h](/media/data/programming/zwergII/src/net/DwarfMessageDispatcher.h#L17).

Fehlend:
- kein `DwarfTrackingController`
- keine Sends auf Modul 7
- keine UI für Objekttracking/Sentry/MOT
- keine Response-/Notify-Verarbeitung für Tracking

Wichtig:
- Das in der Astro-UI vorhandene Sun/Moon-Tracking ist nicht das Tracking-Modul 7, sondern Astro-Cmd `11011/11012`.

Bewertung:
- Das Tracking-Modul der offiziellen API ist aktuell faktisch nicht implementiert.

### 10. Panorama

Implementiert:
- Start/Stop/Progress/State sind vorhanden in [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L12).
- UI ist integriert.

Teilweise / fragil:
- Controller basiert explizit auf PCAP-Reverse-Engineering.
- Zeilen-/Spaltenmapping ist laut TODO nicht verifiziert, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L48).
- Start/Completion werden heuristisch interpretiert, weil `WsPacket.type` im Handler nicht ausgewertet wird, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L246).
- `QThread::msleep` im Ablauf ist ein technisches Warnsignal für UI-/Timing-Festigkeit, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L130).

Abweichend:
- Neben den dokumentierten Panorama-Kommandos werden undokumentierte/inoffizielle Modul-14/15-Kommandos `16402` und `16703` benutzt, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L17).

Bewertung:
- Funktional vorhanden, aber noch nicht sauber „gegen die offizielle API abgesichert“.

### 11. System / RGB / Power

Spezifikation:
- System: `13000` Zeit, `13001` Zeitzone, `13002` MTP, `13003` CPU
- RGB/Power: `13500`-`13505`

Implementiert:
- Neuer Controller für Modul 4/5 mit Request-/Response-Handling in [src/net/DwarfSystemController.cpp](/media/data/programming/zwergII/src/net/DwarfSystemController.cpp).
- `13000` Zeit setzen wird jetzt über den Controller verwendet, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L2135).
- `13001` Zeitzone, `13002` MTP und `13003` CPU sind in der Settings-UI verdrahtet, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L1514).
- `13500`/`13501` RGB-Ring, `13503`/`13504` Power-Indikator sowie `13502` Shutdown und `13505` Reboot sind in derselben UI verdrahtet, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L1514).
- Notifications für RGB-, Power-Indikator-, MTP-, CPU- und Power-Off-Status sind als Proto + Handler ergänzt, siehe [src/proto/notify.proto](/media/data/programming/zwergII/src/proto/notify.proto#L104) und [src/net/DwarfSystemController.cpp](/media/data/programming/zwergII/src/net/DwarfSystemController.cpp#L121).

Fehlend / offen:
- Master-Lock ist weiterhin nicht in die UI eingebunden
- Es gibt keine dedizierte Readback-/Query-Funktion für den initialen Zustand; die UI arbeitet mit Responses und Notifications

Bewertung:
- Modul 4/5 ist für die dokumentierten Kernfunktionen jetzt praktisch abgedeckt.
- Offene Restlücke ist nur noch der nicht priorisierte Master-Lock-/Host-Lock-Pfad.

## Priorisierte Lücken

### Hoch

1. Tracking-Modul 7 komplett implementieren
2. Album-Delete auf dokumentierten API-Pfad prüfen und sauber abbilden
3. Fokus-API auf `15002-15005` erweitern

### Mittel

1. Kamera-Burst/Timelapse für Tele und Wide ergänzen
2. Astro-Controller-Funktionen, die schon vorhanden sind, UI-seitig fertig integrieren
3. Panorama-Protokoll von heuristisch auf verifiziert umstellen
4. Master-Lock aus Modul 4 optional ergänzen

### Niedrig

1. RTSP-Unterstützung ergänzen oder bewusst als „nicht vorgesehen“ dokumentieren
2. Device-HTTP-API vervollständigen (Firmware-Version, Name/Passwort, Upload)

## Gesamturteil

`zwergII` ist kein vollständiger 1:1-Nachbau der DWARF-II-API. Es ist aktuell eher:

- ein gut nutzbarer Qt-Client für den wichtigsten Alltagsumfang,
- mit starker Kamera-/Astro-/Motor-Basis,
- aber mit deutlich unvollständiger Abdeckung der offiziellen API,
- und mit mehreren bewusst pragmatischen Reverse-Engineering-Workarounds.

Für einen „API komplett implementiert“-Status fehlen vor allem:
- Tracking Modul 7
- vollständige Fokus-API
- saubere Device-/Album-HTTP-Abdeckung
- dokumentenkonforme Panorama-/Delete-Pfade
