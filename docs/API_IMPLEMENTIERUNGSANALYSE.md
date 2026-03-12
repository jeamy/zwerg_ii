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
- einzelne Device-/HTTP-Randfunktionen
- Tracking-Modul 7 ist erst teilweise verifiziert
- Fokus-API ist weitgehend, aber nicht vollständig verifiziert
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
| HTTP Device API | Weitgehend | Discovery, Name/Passwort und Firmware-Upload sind ergänzt; Reset/Validierung offen |
| HTTP Album API | Teilweise bis weitgehend | `mediaInfos` und `delete` sind da; `mediaCounts` fehlt |
| JPG Stream | Implementiert | MJPEG `mainstream`/`secondstream` wird genutzt |
| RTSP Stream | Fehlend | In Doku vorhanden, im Qt-Client nicht verwendet |
| Kamera Tele | Weitgehend implementiert | Open/Close/Photo/Burst/Video/Timelapse/Params sind da |
| Kamera Wide | Weitgehend implementiert | Open/Close/Photo/Burst/Timelapse/Params sind da |
| Astro | Weitgehend implementiert | Kernfunktionen, Dark-Frame-Capture, Wide-Stacking und EQ-Solving sind UI-seitig da; einzelne Randkommandos bleiben offen |
| Fokus | Weitgehend implementiert | Continuous Focus und Astro-AF sind ergänzt; Area-AF ist UI-seitig noch nicht genutzt |
| Motor | Weitgehend implementiert | Run/Stop/Joystick/Fixed-Angle/Dual-Linkage sind da |
| Tracking (Modul 7) | Teilweise bis weitgehend | Controller, Notify-Handling und Livebild-UI sind da; einzelne Kommandos sind noch unvalidiert |
| Panorama | Teilweise | Läuft mit robusterer Zustandsführung, bleibt aber reverse-engineered |
| System (Modul 4) | Weitgehend implementiert | Zeit, Zeitzone, MTP und CPU sind als Controller + UI verdrahtet |
| RGB/Power (Modul 5) | Implementiert | RGB-Ring, Power-Indikator, Shutdown und Reboot sind umgesetzt |

## Detaillierte Analyse

### 1. Transport / Dispatcher

Implementiert:
- WebSocket-Client mit Protobuf-WsPacket und Heartbeat ist vorhanden in [src/net/DwarfWebSocketClient.cpp](/media/data/programming/zwergII/src/net/DwarfWebSocketClient.cpp#L137).
- Dispatcher kennt alle relevanten Module 1-15 in [src/net/DwarfMessageDispatcher.h](/media/data/programming/zwergII/src/net/DwarfMessageDispatcher.h#L17).

Teilweise:
- Einige Pfade sind weiter firmwareabhängig oder noch nicht gegen reale Gerätevarianten validiert.

Bewertung:
- Technische Basis ist gut.
- Die Laufzeitverdrahtung deckt jetzt Kamera, Fokus, System, Tracking, Astro, Panorama und HTTP-Device-Aufrufe ab.

### 2. HTTP Device API

Spezifikation:
- `GET/POST /deviceInfo`
- `POST /setDeviceNameAndPsd`
- `POST /firmwareVersion`
- `POST /uploadFirmware`

Implementiert:
- Discovery nutzt jetzt primär den dokumentierten Endpoint `/deviceInfo` in [src/net/DwarfFinder.cpp](/media/data/programming/zwergII/src/net/DwarfFinder.cpp#L218).
- Wenn in der Device-Info keine Version enthalten ist, wird zusätzlich `/firmwareVersion` abgefragt, siehe [src/net/DwarfFinder.cpp](/media/data/programming/zwergII/src/net/DwarfFinder.cpp#L292).
- `POST /setDeviceNameAndPsd` ist im HTTP-Client umgesetzt und im Settings-Tab für Gerätename und Passwort verdrahtet, siehe [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp) und [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp).
- `POST /uploadFirmware` ist als `multipart/form-data` mit MD5-Übertragung umgesetzt und im Settings-Tab verdrahtet, siehe [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp) und [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp).

Fehlt:
- `resetDeviceInfo` ist weiterhin nicht umgesetzt
- Reale Gerätevalidierung für die neuen Schreibpfade fehlt weiterhin

Teilweise / abweichend:
- Für ältere/abweichende Firmwares bleibt `getdeviceInfo` als Fallback erhalten, siehe [src/net/DwarfFinder.cpp](/media/data/programming/zwergII/src/net/DwarfFinder.cpp#L257).
- Beim Firmware-Upload wird zunächst der in der Doku genannte Formularfeldname `fiwmwareFileName` verwendet und bei `invalid parameter` auf `firmwareFileName` gefallbackt, weil die Spezifikation hier uneinheitlich wirkt, siehe [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp).

Bewertung:
- Die dokumentierten Kernendpunkte der Device-HTTP-API sind jetzt praktisch abgedeckt.
- Offen bleiben vor allem Validierung gegen echte Firmware und der nicht priorisierte Reset-Pfad.

### 3. HTTP Album API

Spezifikation:
- `/album/list/mediaCounts`
- `/album/list/mediaInfos`
- `/album/delete`

Implementiert:
- Medienliste via `/album/list/mediaInfos` in [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp#L15).
- Gallery/UI-Klassifikation nach `mediaType` in [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L2140).
- Delete nutzt jetzt primär den dokumentierten Endpoint `/album/delete`, siehe [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp#L89).

Teilweise:
- `mediaCounts` wird nicht verwendet; die UI baut ihre Kategorien aus `mediaInfos` nach.

Abweichend/Falsch:
- Als Firmware-Fallback bleibt `/sdcard/deleteFile` zusätzlich erhalten, falls `/album/delete` auf älteren Geräten fehlt, siehe [src/net/DwarfHttpClient.cpp](/media/data/programming/zwergII/src/net/DwarfHttpClient.cpp#L89).
- Wenn beide HTTP-Wege nicht funktionieren, fällt die App weiter auf USB/MTP zurück, siehe [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L3096) und [src/net/DwarfMtpClient.cpp](/media/data/programming/zwergII/src/net/DwarfMtpClient.cpp#L153).

Bewertung:
- Listen/Thumbnails/Download sind brauchbar.
- Delete ist jetzt spezifikationsnah umgesetzt, mit pragmatischem Legacy-Fallback.

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
- Burst und Timelapse für Tele/Wide sowie zugehörige Notify-Verarbeitung sind jetzt ebenfalls im Controller vorhanden, siehe [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L250).
- Capture-UI für Photo/Record/Burst/Timelapse ist im Panel verdrahtet, siehe [src/ui/CameraSettingsPanel.cpp](/media/data/programming/zwergII/src/ui/CameraSettingsPanel.cpp#L229).
- Exposure/Gain/WB/IR-Cut sowie Bildparameter in [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L313).
- `GET_ALL_PARAMS` und Response-Verarbeitung in [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L582).

Fehlend:
- dedizierte Nutzung von `GET_*`-Einzelparametern
- `GET_SYSTEM_WORKING_STATE` Tele `10039`

Teilweise:
- Parametersteuerung ist breit, aber nicht vollständig dokumentenkonform umgesetzt.

Abweichend/Falsch:
- Für Brightness/Contrast/Hue/Saturation/Sharpness werden nicht die dokumentierten Kamera-Setter verwendet, sondern reverse-engineerte Feature-Parameter über Modul 15 / Cmd 16703, siehe [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L402) und [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L881).
- Video-Start arbeitet mit Firmware-Heuristik und Reopen-Delay statt reinem Spec-Flow, siehe [src/net/DwarfCameraController.cpp](/media/data/programming/zwergII/src/net/DwarfCameraController.cpp#L253).

Bewertung:
- Für normale Nutzung ist die Kameraanbindung stark.
- Die dokumentierten Standard-Capture-Kommandos sind jetzt weitgehend abgedeckt.
- Offene Restlücken liegen primär bei Sonder-/Readback-Kommandos, nicht mehr bei Burst/Timelapse.

### 6. Fokus-API

Spezifikation:
- `15000` Normal-AF
- `15001` Single-Step
- `15002/15003` Continuous Focus
- `15004/15005` Astro AF Start/Stop

Implementiert:
- `15000`/`15001` Normal-AF und Single-Step sind weiter vorhanden in [src/net/DwarfFocusController.cpp](/media/data/programming/zwergII/src/net/DwarfFocusController.cpp#L18).
- `15002/15003` Continuous Focus und `15004/15005` Astro AF Start/Stop sind jetzt ebenfalls im Controller umgesetzt, siehe [src/net/DwarfFocusController.cpp](/media/data/programming/zwergII/src/net/DwarfFocusController.cpp#L64).
- Response- und Notify-Handling für Fokus sind ergänzt, inklusive Positions-Notify `15257`, siehe [src/net/DwarfFocusController.cpp](/media/data/programming/zwergII/src/net/DwarfFocusController.cpp#L99).
- Das Motor-/Fokus-Overlay bietet jetzt Single-Step, Hold-Focus und Astro-AF, siehe [src/ui/MotorControlPanel.cpp](/media/data/programming/zwergII/src/ui/MotorControlPanel.cpp#L196).

Offen:
- Area-AF-Variante von `15000` mit expliziten Koordinaten ist UI-seitig noch nicht exponiert
- Keine Gerätevalidierung der neuen Fokusmodi erfolgt

Bewertung:
- Die dokumentierten Fokus-Kommandos `15000-15005` sind jetzt im Client abgedeckt.
- Restoffen ist vor allem die tiefergehende Bedienung/Validierung, nicht mehr die reine API-Abwesenheit.

### 7. Astro-API

Implementiert:
- Controller deckt große Teile des Astro-Moduls ab: Kalibrierung, GOTO/One-Click-GOTO, `Go Live`, Stacking, Wide-Stacking, Darkframes, EQ-Solving, Special-Target-Tracking, siehe [src/net/DwarfAstroController.h](/media/data/programming/zwergII/src/net/DwarfAstroController.h#L18) und [src/net/DwarfAstroController.cpp](/media/data/programming/zwergII/src/net/DwarfAstroController.cpp#L18).
- Notifications für Akku, SD, Stacking, Kalibrierung, GOTO, Temperatur werden verarbeitet in [src/net/DwarfAstroController.cpp](/media/data/programming/zwergII/src/net/DwarfAstroController.cpp#L518).
- UI nutzt GoLive + Kalibrierung + One-Click-GOTO + Live-Stacking aktiv, siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1345) und [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1462).
- Dark-Frame-Capture ist jetzt auch im Astro-Panel verdrahtet, inklusive Fortschritt und gespeicherter Profilanzeige, siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp).
- Wide-vs.-Tele-Stacking ist jetzt in der Astro-UI auswählbar und nutzt die passenden Kamera-Parameter sowie den passenden Astro-Command, siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp).
- EQ-Solving ist jetzt im Settings-Tab mit Start/Stop und Fehleranzeige verdrahtet, siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp).

Teilweise:
- Flat-/Bias-Frame-Bedienelemente waren vorhanden, werden jetzt aber bewusst als nicht unterstützt behandelt, solange es dafür keinen sauberen Controller/API-Pfad gibt.
- `DEL_DARK_FRAME_LIST` ist im Controller gar nicht umgesetzt, obwohl der Cmd-Block definiert ist, siehe [src/net/DwarfAstroController.cpp](/media/data/programming/zwergII/src/net/DwarfAstroController.cpp#L40).

Abweichend:
- Mehrere Abläufe arbeiten mit zusätzlichen Delays und heuristischen Vorbedingungen (`Go Live`, Kameras öffnen, Kalibrierung abwarten), siehe [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1349) und [src/ui/AstroNavigationPanel.cpp](/media/data/programming/zwergII/src/ui/AstroNavigationPanel.cpp#L1515).

Bewertung:
- Astro ist eines der stärksten Module im Projekt.
- Die wesentliche Tele-Astro-Bedienung ist jetzt weitgehend UI-seitig abgedeckt.
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
- Dual-Camera-Linkage `14009` ist jetzt ebenfalls im Controller vorhanden, siehe [src/net/DwarfMotorController.cpp](/media/data/programming/zwergII/src/net/DwarfMotorController.cpp#L102).
- UI bietet einen Linkage-Modus im Motor-Overlay; ein Klick auf den Wide-Stream sendet dann `14009`, siehe [src/ui/MotorControlPanel.cpp](/media/data/programming/zwergII/src/ui/MotorControlPanel.cpp#L165) und [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L2270).

Offen:
- Kein dediziertes Response-Parsing des Motor-Moduls; `14009` arbeitet aktuell wie mehrere andere Bewegungsbefehle fire-and-forget

Bewertung:
- Bewegungssteuerung ist nutzbar.
- Die dokumentierten Motor-Kommandos sind jetzt praktisch vollständig abgedeckt.

### 9. Tracking-Modul 7

Spezifikation:
- `14800` Start Track
- `14801` Stop Track
- `14802/14803` Sentry
- `14804/14805` MOT
- weitere dokumentierte Tracking-Kommandos bis `14810`

Implementiert:
- Vollständiger Tracking-Controller für `14800-14810` in [src/net/DwarfTrackingController.cpp](/media/data/programming/zwergII/src/net/DwarfTrackingController.cpp).
- Erweiterte Tracking-Protos für `14809/14810` in [src/proto/tracking.proto](/media/data/programming/zwergII/src/proto/tracking.proto#L1).
- Notify-Handling für Track-Result, Sentry/UFO-State und Multi-Track-Result in [src/proto/notify.proto](/media/data/programming/zwergII/src/proto/notify.proto#L126).
- Livebild-Box-Auswahl und Ergebnis-Overlay in [src/ui/TrackingOverlayWidget.cpp](/media/data/programming/zwergII/src/ui/TrackingOverlayWidget.cpp).
- Bedien-Overlay im Hauptfenster für Objekttracking, Sentry, UFO, MOT, Source-Switch und UFO-Hand/Auto in [src/MainWindow.cpp](/media/data/programming/zwergII/src/MainWindow.cpp#L1010).

Offen / unsicher:
- `14809` Wide/Tele-Switch und `14810` UFO hand/auto basieren auf der Doku-Inferenz `int32 mode`, sind aber noch nicht gegen reale Firmware validiert
- Es gibt noch keine separate Verifikation, welche Tracking-Modi auf Tele bzw. Wide in allen Firmware-Versionen tatsächlich akzeptiert werden
- Keine Laufzeitverifikation gegen ein echtes Gerät durchgeführt

Wichtig:
- Das in der Astro-UI vorhandene Sun/Moon-Tracking ist nicht das Tracking-Modul 7, sondern Astro-Cmd `11011/11012`.

Bewertung:
- Modul 7 ist jetzt funktional im Client vertreten und bedienbar.
- Vollständig belastbar ist die Umsetzung erst nach Gerätevalidierung der letzten Randkommandos.

### 10. Panorama

Implementiert:
- Start/Stop/Progress/State sind vorhanden in [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L12).
- UI ist integriert.
- Grid-Updates laufen jetzt entblockt und koalesziert per Timer statt mit blockierendem `msleep`, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp).
- Start-/Stop-Acks werden jetzt mit Pending-State robuster unterschieden, sodass Completion nicht mehr nur an einem nackten Wiederauftreten von `15500` hängt, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp).

Teilweise / fragil:
- Controller basiert explizit auf PCAP-Reverse-Engineering.
- Zeilen-/Spaltenmapping ist laut TODO nicht verifiziert, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L48).
- Die Auswertung bleibt weiterhin ohne direkten `WsPacket.type`-Kontext und ist daher noch nicht vollständig protokollsauber.

Abweichend:
- Neben den dokumentierten Panorama-Kommandos werden undokumentierte/inoffizielle Modul-14/15-Kommandos `16402` und `16703` benutzt, siehe [src/net/DwarfPanoramaController.cpp](/media/data/programming/zwergII/src/net/DwarfPanoramaController.cpp#L17).

Bewertung:
- Funktional vorhanden und deutlich robuster als zuvor.
- Vollständig gegen die offizielle API abgesichert ist der Ablauf wegen des reverse-engineerten Grid-Protokolls aber weiter nicht.

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

1. Tracking-Modul 7 gegen reale Firmware validieren, insbesondere `14809/14810`
2. Fokus- und Tracking-Neuerungen gegen reale Firmware validieren
3. Device-HTTP-Schreibpfade gegen reale Firmware validieren

### Mittel

1. `DEL_DARK_FRAME_LIST` und weitere Astro-Randkommandos ergänzen
2. Panorama-Row/Col-Mapping gegen reale Firmware verifizieren
3. Master-Lock aus Modul 4 optional ergänzen
4. `resetDeviceInfo` aus der erweiterten Device-Doku optional ergänzen

### Niedrig

1. RTSP-Unterstützung ergänzen oder bewusst als „nicht vorgesehen“ dokumentieren
2. `mediaCounts` ergänzen, falls die Gallery künftig Counts vorab braucht

## Gesamturteil

`zwergII` ist kein vollständiger 1:1-Nachbau der DWARF-II-API. Es ist aktuell eher:

- ein gut nutzbarer Qt-Client für den wichtigsten Alltagsumfang,
- mit starker Kamera-/Astro-/Motor-Basis,
- aber mit deutlich unvollständiger Abdeckung der offiziellen API,
- und mit mehreren bewusst pragmatischen Reverse-Engineering-Workarounds.

Für einen „API komplett implementiert“-Status fehlen vor allem:
- saubere Device-/Album-HTTP-Abdeckung
- dokumentenkonforme Panorama-/Delete-Pfade
