#!/usr/bin/env python3
"""Einfache Auswertung der von dwarf_capture.py erzeugten JSON-Logs.

Fasst für alle Dateien vom Muster dwarf_capture_*.json im Capture-Verzeichnis
WS- und HTTP-Ergebnisse zusammen und schreibt eine Text-Zusammenfassung auf STDOUT.

Schwerpunkte:
- Übersicht über Anzahl / Erfolgsrate der WS-Kommandos nach Name.
- Übersicht über HTTP-Endpunkte nach Name + Status-Code.
- Spezielle Auflistung für Astro-GOTO-Kommandos und Darkframe-Löschkommandos.
- Spezielle Auflistung für Motor-Kommandos.
"""

import json
import os
import sys
from collections import Counter, defaultdict
from typing import Any, Dict, List

HERE = os.path.abspath(os.path.dirname(__file__))
REPO_ROOT = os.path.dirname(HERE)
CAPTURE_DIR = os.path.join(REPO_ROOT, "capture")


def find_log_files() -> List[str]:
    files: List[str] = []
    if not os.path.isdir(CAPTURE_DIR):
        return files
    for name in os.listdir(CAPTURE_DIR):
        if name.startswith("dwarf_capture_") and name.endswith(".json"):
            files.append(os.path.join(CAPTURE_DIR, name))
    return sorted(files)


def load_log(path: str) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def summarize_logs(logs: List[Dict[str, Any]]) -> None:
    if not logs:
        print("Keine Logdateien gefunden.")
        return

    ws_by_name: Counter = Counter()
    ws_errors_by_name: Counter = Counter()
    ws_detail_codes: Dict[str, Counter] = defaultdict(Counter)

    http_by_name: Counter = Counter()
    http_status_by_name: Dict[str, Counter] = defaultdict(Counter)

    astro_goto_entries: List[Dict[str, Any]] = []
    dark_delete_entries: List[Dict[str, Any]] = []
    motor_entries: List[Dict[str, Any]] = []
    pano_entries: List[Dict[str, Any]] = []

    for log in logs:
        ws_results = log.get("ws_results") or []
        for ws in ws_results:
            name = str(ws.get("name", "<unknown>"))
            ws_by_name[name] += 1
            if "error" in ws or ws.get("response_code") not in (None, 0):
                ws_errors_by_name[name] += 1
            code = ws.get("response_code")
            if code is not None:
                ws_detail_codes[name][code] += 1

            if name.startswith("astro_goto") or name.startswith("astro_one_click_goto"):
                astro_goto_entries.append(ws)
            if "dark_frame" in name or "dark_frame_list" in name:
                dark_delete_entries.append(ws)
            if name.startswith("motor_"):
                motor_entries.append(ws)
            if name.startswith("panorama_"):
                pano_entries.append(ws)

        http_results = log.get("http_results") or []
        for ep in http_results:
            name = str(ep.get("name", "<unknown>"))
            http_by_name[name] += 1
            status = ep.get("status_code")
            if status is None:
                http_status_by_name[name]["error/none"] += 1
            else:
                http_status_by_name[name][str(status)] += 1

    print("=== Zusammenfassung WS-Kommandos (über alle Logs) ===")
    for name, total in ws_by_name.most_common():
        errors = ws_errors_by_name.get(name, 0)
        code_counts = ws_detail_codes.get(name)
        code_str = ""
        if code_counts:
            parts = []
            for c, n in sorted(code_counts.items()):
                label = "OK" if c == 0 else "ERROR"
                parts.append(f"{c} ({label}) -> {n}")
            code_str = ", Codes: " + ", ".join(parts)
        print(f"- {name}: {total} Aufruf(e), Fehler: {errors}{code_str}")

    print("\n=== Zusammenfassung HTTP-Endpunkte (über alle Logs) ===")
    for name, total in http_by_name.most_common():
        statuses = http_status_by_name.get(name)
        if statuses:
            status_str = ", ".join(
                f"{code}: {cnt}" for code, cnt in sorted(statuses.items())
            )
        else:
            status_str = "(keine Status-Angaben)"
        print(f"- {name}: {total} Aufruf(e); Status: {status_str}")

    print("\n=== Astro-GOTO / One-Click-GOTO Details ===")
    if not astro_goto_entries:
        print("(keine Einträge)")
    else:
        for e in astro_goto_entries:
            hdr = e.get("response_header", {})
            print(
                f"- {e.get('name')} cmd={e.get('cmd')} code={e.get('response_code')} "
                f"hdr_type={hdr.get('type')}"
            )

    print("\n=== Darkframe-Löschkommandos (Tele + Wide) ===")
    if not dark_delete_entries:
        print("(keine Einträge)")
    else:
        for e in dark_delete_entries:
            print(f"- {e.get('name')} cmd={e.get('cmd')} code={e.get('response_code')}")

    print("\n=== Motor-Kommandos (GOTO / Joystick / Stop) ===")
    if not motor_entries:
        print("(keine Einträge)")
    else:
        for e in motor_entries:
            print(f"- {e.get('name')} cmd={e.get('cmd')} code={e.get('response_code')}")

    print("\n=== Panorama-Kommandos ===")
    if not pano_entries:
        print("(keine Einträge)")
    else:
        for e in pano_entries:
            print(f"- {e.get('name')} cmd={e.get('cmd')} code={e.get('response_code')}")


def main() -> None:
    files = find_log_files()
    if not files:
        print(f"Kein Log gefunden im Verzeichnis: {CAPTURE_DIR}")
        sys.exit(0)

    print("Analysiere folgende Logs:")
    for p in files:
        print(f"  - {os.path.basename(p)}")

    logs = [load_log(p) for p in files]
    summarize_logs(logs)


if __name__ == "__main__":
    main()
