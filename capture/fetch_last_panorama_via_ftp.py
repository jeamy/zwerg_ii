#!/usr/bin/env python3
"""Hole das letzte Panorama-Bild per FTP, sofern es zum Aufnahmedatum passt.

Logik:
- Lies eine dwarf_capture_*.json-Logdatei (Standard: die zuletzt erstellte).
- Nimm das Datum aus meta.started_at.
- Verbinde dich per FTP mit dem DWARF-II, gehe in das Panorama-Verzeichnis
  (Standard: /Panorama, per CLI konfigurierbar).
- Ermittle die jüngste Datei (per MLSD-"modify" oder über den Dateinamen).
- Wenn deren Datum mit dem Aufnahmedatum übereinstimmt, lade sie herunter.

Hinweis:
- FTP-User/Passwort und das Verzeichnis sind CLI-Argumente; Standard ist
-  anonyme Anmeldung und /DWARF_II/Panoramas.
- Dieses Skript verändert nichts auf dem Gerät, es liest nur Dateien.
"""

import argparse
import datetime as dt
import json
import os
import re
from ftplib import FTP, error_perm
from typing import Any, Dict, List, Optional, Tuple

HERE = os.path.abspath(os.path.dirname(__file__))
REPO_ROOT = os.path.dirname(HERE)
CAPTURE_DIR = os.path.join(REPO_ROOT, "capture")


def find_latest_log(path: str) -> Optional[str]:
    files: List[str] = []
    if not os.path.isdir(path):
        return None
    for name in os.listdir(path):
        if name.startswith("dwarf_capture_") and name.endswith(".json"):
            files.append(os.path.join(path, name))
    if not files:
        return None
    # Nach Dateiname sortiert (YYYYMMDD_HHMMSS im Namen)
    files.sort()
    return files[-1]


def load_capture_date(log_path: str) -> dt.date:
    with open(log_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    started = data.get("meta", {}).get("started_at")
    if not started:
        raise RuntimeError("meta.started_at fehlt im Log")
    # ISO-8601 nach datetime parsen
    ts = dt.datetime.fromisoformat(started.replace("Z", "+00:00"))
    return ts.date()


def connect_ftp(host: str, user: str, password: str, timeout: float) -> FTP:
    ftp = FTP()
    ftp.connect(host=host, timeout=timeout)
    ftp.login(user=user, passwd=password)
    return ftp


def parse_date_from_modify(modify: str) -> Optional[dt.date]:
    """Erwarte format YYYYMMDDHHMMSS von MLSD."""
    if len(modify) < 8:
        return None
    try:
        return dt.datetime.strptime(modify[:8], "%Y%m%d").date()
    except ValueError:
        return None


FILENAME_DATE_RE = re.compile(r"DWARF_(\d{4})(\d{2})(\d{2})")

# Bekannte Galerie-Verzeichnisse auf dem DWARF-II
GALLERY_DIRS = [
    "/DWARF_II/Astronomy",
    "/DWARF_II/Burst",
    "/DWARF_II/data",
    "/DWARF_II/Normal_Photos",
    "/DWARF_II/Panoramas",
    "/DWARF_II/Videos",
]


def parse_date_from_filename(name: str) -> Optional[dt.date]:
    m = FILENAME_DATE_RE.search(name)
    if not m:
        return None
    y, mth, d = m.groups()
    try:
        return dt.date(int(y), int(mth), int(d))
    except ValueError:
        return None


def find_latest_file_in_dir(
    ftp: FTP,
    directory: str,
) -> Optional[Tuple[str, Optional[dt.date]]]:
    """Finde die jüngste Datei in einem FTP-Verzeichnis.

    Versucht zuerst MLSD (mit modify-Attribut), fällt sonst auf Namen/Sortierung
    zurück.
    """
    try:
        ftp.cwd(directory)
    except error_perm as exc:
        raise RuntimeError(f"FTP-Verzeichnis nicht gefunden: {directory!r}: {exc}") from exc

    candidates: List[Tuple[str, Optional[dt.date]]] = []

    # Bevorzugt MLSD nutzen, falls unterstützt
    try:
        for facts, name in ftp.mlsd():
            if facts.get("type") != "file":
                continue
            modify = facts.get("modify")
            file_date = parse_date_from_modify(modify) if modify else None
            candidates.append((name, file_date))
    except (error_perm, AttributeError):
        # MLSD nicht verfügbar, Fallback: einfache Liste + Datum aus Name
        names = ftp.nlst()
        for name in names:
            # Verzeichnisse ignorieren (heuristisch: Einträge ohne Punkt sind
            # wahrscheinlich Verzeichnisse)
            if "." not in name:
                continue
            file_date = parse_date_from_filename(name)
            candidates.append((name, file_date))

    if not candidates:
        return None

    # Sortiere nach (Datum, Name); None-Datums kommen nach hinten
    def sort_key(item: Tuple[str, Optional[dt.date]]):
        name, d = item
        return (d or dt.date.min, name)

    candidates.sort(key=sort_key)
    return candidates[-1]


def download_file(ftp: FTP, remote_name: str, local_path: str) -> None:
    os.makedirs(os.path.dirname(local_path), exist_ok=True)
    with open(local_path, "wb") as f:
        ftp.retrbinary(f"RETR {remote_name}", f.write)


def count_files_in_dir(ftp: FTP, directory: str) -> Optional[int]:
    """Zähle Dateien in einem Verzeichnis (nur Files, keine Unterverzeichnisse)."""
    try:
        ftp.cwd(directory)
    except error_perm:
        return None

    count = 0
    try:
        for facts, name in ftp.mlsd():
            if facts.get("type") == "file":
                count += 1
    except (error_perm, AttributeError):
        # Fallback: einfache Liste, Heuristik: Einträge mit Punkt sind Dateien
        try:
            names = ftp.nlst()
        except error_perm:
            return None
        for name in names:
            if "." in name:
                count += 1
    return count


def print_gallery_overview(ftp: FTP) -> None:
    print("FTP Galerie-Übersicht:")
    for d in GALLERY_DIRS:
        cnt = count_files_in_dir(ftp, d)
        if cnt is None:
            print(f"  {d}: NICHT ERREICHBAR")
        else:
            print(f"  {d}: {cnt} Dateien")

def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description=(
            "Letztes Panorama-Bild per FTP holen, wenn es zum Capture-Log-Datum passt."
        )
    )
    p.add_argument("--host", required=True, help="DWARF-II Host/IP (FTP-Server)")
    p.add_argument(
        "--ftp-user",
        default="anonymous",
        help="FTP-Benutzername (Standard: anonymous)",
    )
    p.add_argument(
        "--ftp-pass",
        default="",
        help="FTP-Passwort (Standard: leer für anonymous)",
    )
    p.add_argument(
        "--ftp-dir",
        default="/DWARF_II/Panoramas",
        help="FTP-Verzeichnis der Panorama-Gallery (Standard: /DWARF_II/Panoramas)",
    )
    p.add_argument(
        "--log-file",
        default=None,
        help=(
            "Pfad zu dwarf_capture_*.json. Wenn nicht angegeben, wird die "
            "zuletzt erstellte Logdatei im capture-Verzeichnis verwendet."
        ),
    )
    p.add_argument(
        "--timeout",
        type=float,
        default=10.0,
        help="FTP-Timeout in Sekunden",
    )
    p.add_argument(
        "--out-dir",
        default=os.path.join(CAPTURE_DIR, "panorama_downloads"),
        help="Lokales Zielverzeichnis für Downloads",
    )
    return p


def main() -> None:
    parser = build_arg_parser()
    args = parser.parse_args()

    log_path = args.log_file
    if not log_path:
        log_path = find_latest_log(CAPTURE_DIR)
        if not log_path:
            raise SystemExit("Keine dwarf_capture_*.json-Logs im capture-Verzeichnis gefunden.")
    print(f"Verwende Logdatei: {log_path}")

    capture_date = load_capture_date(log_path)
    print(f"Aufnahmedatum aus Log: {capture_date.isoformat()}")

    ftp = connect_ftp(args.host, args.ftp_user, args.ftp_pass, args.timeout)
    try:
        # Zuerst Galerie-Übersicht ausgeben
        print_gallery_overview(ftp)
        latest = find_latest_file_in_dir(ftp, args.ftp_dir)
        if not latest:
            print(f"Keine Dateien in FTP-Verzeichnis {args.ftp_dir!r} gefunden.")
            return
        remote_name, file_date = latest
        print(f"Letzte Datei: {remote_name}, Datum: {file_date}")

        if file_date is None:
            print("Konnte kein Datum für die letzte Datei bestimmen; kein Download.")
            return

        if file_date != capture_date:
            print(
                "Datum stimmt nicht überein: "
                f"Datei={file_date.isoformat()}, Capture={capture_date.isoformat()}"
            )
            return

        local_path = os.path.join(args.out_dir, remote_name)
        print(f"Lade Datei nach: {local_path}")
        download_file(ftp, remote_name, local_path)
        print("Download abgeschlossen.")
    finally:
        try:
            ftp.quit()
        except Exception:
            try:
                ftp.close()
            except Exception:
                pass


if __name__ == "__main__":
    main()
