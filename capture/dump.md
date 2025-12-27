# Dump & Analyse Workflow (tcpdump/tshark)

## Ziel
Aus einer Capture-Datei (pcapng) die *relevanten* Befehle für:
- `rows/cols` (Panorama-Grid / Preset)
- `panorama start`
- `panorama stop`

extrahieren – auch wenn die Nutzlast **nicht als Klartext** vorliegt.

---

## 0) Voraussetzungen
- Interface zum DWARF-Netz: z.B. `wlp47s0f3u4`
- DWARF IP: `10.42.0.209`
- Capture-Ordner im Repo: `capture/`

---

## 1) Capture aufnehmen (tcpdump)

### 1.1 Control-only Capture (kein MJPEG-Stream-Port 8092)
Im Repo-Root ausführen:

```bash
mkdir -p capture
TS=$(date +%Y%m%d_%H%M%S)

sudo tcpdump -i wlp47s0f3u4 -s 0 -nn \
  -w capture/ctrl_${TS}.pcapng \
  'host 10.42.0.209 and tcp and not port 8092'
```

Stop: `Ctrl+C`

### 1.2 Empfohlener Ablauf während der Aufnahme
Für eindeutige Zuordnung (später Diff):
- `rows/cols` ändern (z.B. 3x3 -> 4x4)
- `panorama start`
- 1–2 kurze Aufnahmen/Actions
- `panorama stop`

Zwischen Aktionen jeweils 2–5 Sekunden warten.

---

## 2) Ports / Streams finden (tshark)

```bash
tshark -r capture/ctrl_YYYYmmdd_HHMMSS.pcapng -q -z conv,tcp
```

Typisch ist ein einziger Stream auf Port `9900`.

---

## 3) WebSocket-Traffic auf Port 9900 extrahieren

### 3.1 Client -> DWARF (Commands)
```bash
tshark -r capture/ctrl_YYYYmmdd_HHMMSS.pcapng \
  -Y "tcp.dstport==9900 && tcp.len>0" \
  -T fields -E separator=$'\t' \
  -e frame.number -e frame.time_relative -e tcp.len -e data
```

### 3.2 DWARF -> Client (Responses)
```bash
tshark -r capture/ctrl_YYYYmmdd_HHMMSS.pcapng \
  -Y "tcp.srcport==9900 && tcp.len>0" \
  -T fields -E separator=$'\t' \
  -e frame.number -e frame.time_relative -e tcp.len -e data
```

---

## 4) Interpretation: WebSocket-Frames erkennen

In den Hex-Daten sieht man WebSocket-Frame-Typen:
- `0x81` = Text Frame
- `0x82` = Binary Frame
- `0x89` = Ping
- `0x8a` = Pong

Wichtig:
- Client->Server Frames sind in WebSocket **maskiert**.
- Server->Client Frames sind **unmaskiert**.

Darum muss man für Client-Commands die Payload **unmasken**, bevor man sinnvoll diffen kann.

---

## 5) WebSocket-Payload unmasken (Python, keine Dependencies)

### 5.1 Unmask-Helfer
```bash
python3 - <<'PY'
def unmask_ws(hexstr):
    b = bytes.fromhex(hexstr)
    fin_opcode = b[0]
    mask_len = b[1]
    masked = (mask_len & 0x80) != 0
    ln = (mask_len & 0x7f)
    i = 2
    if ln == 126:
        ln = int.from_bytes(b[i:i+2], 'big'); i += 2
    elif ln == 127:
        ln = int.from_bytes(b[i:i+8], 'big'); i += 8
    if masked:
        mask = b[i:i+4]; i += 4
        payload = bytearray(b[i:i+ln])
        for k in range(ln):
            payload[k] ^= mask[k % 4]
        return fin_opcode, bytes(payload)
    else:
        return fin_opcode, b[i:i+ln]

# Beispiel: hier die Hex-Frames aus tshark einsetzen
frames = {
  # frame_number: "<hex aus tshark data>"
}

for fn in sorted(frames.keys()):
    op, payload = unmask_ws(frames[fn])
    print(f"FRAME {fn}: opcode=0x{op:02x} payload_len={len(payload)}")
    print(payload.hex())
    print()
PY
```

### 5.2 Typisches Ergebnis
Nach dem Unmasken sieht man *stabile* Payloads, bei denen für bestimmte Aktionen nur wenige Bytes abweichen:
- `rows/cols` Änderung: 2 Bytes ändern sich (oft +1)
- `pano start/stop`: 1 Byte unterscheidet sich (Command-ID)

---

## 6) Diff-Workflow: rows/cols vs pano start/stop identifizieren

### 6.1 Candidate-Frames finden
Im Client->Server Dump:
- Kleine 10-Byte Textframes (`81 84 ...`) sind meist Poll/Keepalive.
- Relevante Commands sind meist größere Binaryframes (`82 ...`).

### 6.2 Unmaskte Payloads diffen
- `rows/cols`: vergleiche zwei 64-Byte Payloads vor/nach Änderung
- `pano start/stop`: vergleiche zwei ~49-Byte Payloads

Praktisch:
- Wenn Payloads bis auf 1–2 Bytes identisch sind, ist das genau das gesuchte Command-Delta.

---

## 7) (Optional) Capture-Strategie für 100% eindeutiges Mapping
Wenn UI nur Presets kann oder du exakte Semantik brauchst:
- mache 2–3 **separate** Captures (idle / grid-set / pano-start / pano-stop)
- diff nur die Client->Server WebSocket Binary Commands

---

## 7.1 Automatisierung: ws_extract.py
Im Repo liegt ein kleines Script, das den kompletten Ablauf automatisiert:
- `tshark` Extract (Client->Server dstport 9900)
- WebSocket-Frames unmasken
- nur Binary Commands (`0x82`) sammeln
- Summary + Diffs ausgeben

### Beispiele

Ein einzelnes Capture analysieren:
```bash
python3 capture/ws_extract.py capture/ctrl_YYYYmmdd_HHMMSS.pcapng --host 10.42.0.209
```

Zwei Captures (z.B. row vs col) analysieren und zusätzlich den ersten Binary-Command gegeneinander diffen:
```bash
python3 capture/ws_extract.py \
  capture/ctrl_ROW.pcapng capture/ctrl_COL.pcapng \
  --host 10.42.0.209 \
  --diff-first-two
```

Wenn du die kompletten unmaskten Payloads brauchst:
```bash
python3 capture/ws_extract.py capture/ctrl_YYYYmmdd_HHMMSS.pcapng --host 10.42.0.209 --show-payload
```

---

## 8) Beispiel: Row/Col-Änderungen (ctrl_20251225_120714.pcapng)

### 8.1 Client->Server Dump erzeugen
```bash
tshark -r capture/ctrl_20251225_120714.pcapng \
  -Y "tcp.dstport==9900 && tcp.len>0" \
  -T fields -E separator=$'\t' \
  -e frame.number -e frame.time_relative -e tcp.len -e data \
  > /tmp/c2s_120714.tsv
```

### 8.2 Unmask + Diff (abwechselnd Row/Col geändert)
In diesem Capture wurden `row` und `col` abwechselnd geändert. Die Analyse liefert:

```text
Binary commands: 4
- frame   4 t=   2.788s payload_len=64 head=080110141801200f28bf8201
- frame  14 t=   6.618s payload_len=64 head=080110141801200f28bf8201
- frame  19 t=   9.285s payload_len=64 head=080110141801200f28bf8201
- frame  29 t=  12.370s payload_len=64 head=080110141801200f28bf8201

Diffs between consecutive binary commands:
4->14  changes=2  [15:9c->9d] [25:04->05]
14->19 changes=1  [15:9d->9c]
19->29 changes=2  [15:9c->9d] [25:05->07]
```

### 8.3 Interpretation (praktisch)
- Offset **15** toggelt (`0x9c` <-> `0x9d`) und korreliert mit dem *abwechselnden* Ändern von `row`/`col`.
- Offset **25** trägt sehr wahrscheinlich den *numerischen Wert* (Index/Anzahl) und ändert sich passend zu den gewählten Einstellungen.

**Finales Mapping (bestätigt durch separate Captures):**
- Offset **15**:
  - `0x9c` = **row**
  - `0x9d` = **col**
- Offset **25**: **Wert/Index** für die jeweils selektierte Dimension (row oder col)

### 8.4 Minimaler Validierungstest (Row vs Col eindeutig)
Ziel: feststellen, ob `0x9c` oder `0x9d` für `row` bzw. `col` steht.

1. Neues kurzes Capture starten
2. **Nur `row`** einmal ändern (ohne `col` anzufassen)
3. Stop
4. Den gleichen Diff-Workflow ausführen

Dann wiederholen für **nur `col`**.

Erwartung:
- Bei „nur row“ bleibt das Toggle-Byte (Offset 15) konstant auf einem Wert (z.B. immer `0x9c`), während Offset 25 sich ändert.
- Bei „nur col“ bleibt Offset 15 konstant auf dem anderen Wert (z.B. `0x9d`).

Wichtig:
- Der Capture muss **mindestens 2 WebSocket-Binary-Commands** enthalten (Baseline + Änderung), sonst gibt es keinen Diff.
- Praktisch sicherstellen, indem du im Capture-Fenster **zweimal** verstellst (z.B. `row +1`, kurz warten, dann `row -1` oder nochmal `row +1`).
- Alternativ: erst einmal bewusst einen bekannten Baseline-Wert setzen, warten, dann den Zielwert setzen.

### 8.5 Bestätigungsbeispiele (row-only vs col-only)

#### Row-only: ctrl_20251225_121525.pcapng
```text
Binary commands: 4
- frame    6 t=   4.453s payload_len=64 head=080110141801200f28bf8201
- frame   17 t=   8.729s payload_len=64 head=080110141801200f28bf8201
- frame   25 t=  13.931s payload_len=64 head=080110141801200f28bf8201
- frame   36 t=  17.454s payload_len=64 head=080110141801200f28bf8201

Diffs between consecutive binary commands:
6->17 changes=1  [25:05->07]
17->25 changes=1 [25:07->09]
25->36 changes=1 [25:09->0a]
```

#### Col-only: ctrl_20251225_121548.pcapng
```text
Binary commands: 5
- frame    4 t=   1.190s payload_len=64 head=080110141801200f28bf8201
- frame    9 t=   4.483s payload_len=64 head=080110141801200f28bf8201
- frame   17 t=   7.007s payload_len=64 head=080110141801200f28bf8201
- frame   22 t=   9.811s payload_len=64 head=080110141801200f28bf8201
- frame   29 t=  11.571s payload_len=64 head=080110141801200f28bf8201

Diffs between consecutive binary commands:
4->9 changes=1  [25:05->07]
9->17 changes=1 [25:07->09]
17->22 changes=1 [25:09->0b]
```

#### Selector-Diff (row vs col)
```text
Diff between first binary command of each capture:
ctrl_20251225_121525.pcapng (row)  vs  ctrl_20251225_121548.pcapng (col)
[15:9c->9d]
```

---

## Status
Dieses Dokument beschreibt den exakten Ablauf (tcpdump -> tshark -> WS-Erkennung -> Unmask -> Diff), wie er in der Analyse verwendet wurde.
