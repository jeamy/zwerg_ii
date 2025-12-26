#!/usr/bin/env python3
"""DWARF II capture script.

Ruft die DWARF-II-HTTP- und WebSocket-APIs auf und loggt die Antworten in eine
JSON-Datei im Capture-Verzeichnis.

Hinweis:
- Die IP/der Host des Teleskops wird NICHT hardcodiert. Verwende
  `--host` oder die Umgebungsvariable `DWARF_HOST`.
- Der Firmware-Upload-Endpunkt wird bewusst NICHT verwendet und kommt im
  Script nicht vor.
- Es gibt Flags, um potentiell destruktive/gefährliche Kommandos zu
  aktivieren (z. B. Reset, Motorbewegung, GOTO, Power-Down).
"""

import argparse
import datetime as _dt
import json
import os
import sys
import time
from typing import Any, Callable, Dict, List, Optional

import requests
from ftplib import FTP, error_perm
from websocket import WebSocket, create_connection
import sqlite3


HERE = os.path.abspath(os.path.dirname(__file__))
REPO_ROOT = os.path.dirname(HERE)  # /media/data/programming/zwergII

# Galerie-Verzeichnisse auf dem DWARF-II für FTP-Checks
GALLERY_DIRS = [
    "/DWARF_II/Astronomy",
    "/DWARF_II/Burst",
    "/DWARF_II/data",
    "/DWARF_II/Normal_Photos",
    "/DWARF_II/Panoramas",
    "/DWARF_II/Videos",
]

# Eigene, dynamisch definierte Protobuf-Messages für die DWARF-API
import dwarf_proto_runtime as proto


# Modul-IDs laut Doku
MODULE_CAMERA_TELE = 1
MODULE_CAMERA_WIDE = 2
MODULE_ASTRO = 3
MODULE_SYSTEM = 4
MODULE_RGB_POWER = 5
MODULE_MOTOR = 6
MODULE_TRACK = 7  # aktuell keine Requests implementiert
MODULE_FOCUS = 8
MODULE_NOTIFY = 9  # nur Notifications
MODULE_PANORAMA = 10  # aktuell keine Requests implementiert
MODULE_SHOOTING_SCHEDULE = 13  # aktuell keine Requests implementiert


def _now_iso() -> str:
    # timezone-aware UTC-Zeit, vermeidet DeprecationWarning für utcnow()
    return _dt.datetime.now(_dt.timezone.utc).isoformat()


def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="DWARF II API Capture Script")
    p.add_argument(
        "--host",
        dest="host",
        help=(
            "Hostname oder IP des DWARF-II (statt im Dokument maskierter Adresse). "
            "Alternativ: Umgebungsvariable DWARF_HOST."
        ),
    )
    p.add_argument(
        "--timeout",
        type=float,
        default=8.0,
        help="Netzwerk-Timeout in Sekunden (HTTP und WebSocket)",
    )
    p.add_argument(
        "--skip-ws",
        action="store_true",
        help="WebSocket-Kommandos überspringen (nur HTTP).",
    )
    p.add_argument(
        "--skip-http",
        action="store_true",
        help="HTTP-Endpunkte überspringen (nur WebSocket).",
    )
    p.add_argument(
        "--include-dangerous-ws",
        action="store_true",
        help=(
            "Gefährliche WS-Kommandos aktivieren (z. B. GOTO, Dark-Frames löschen, "
            "EQ-Verify, evtl. größere Motorbewegungen)."
        ),
    )
    p.add_argument(
        "--include-dangerous-http",
        action="store_true",
        help=(
            "Gefährliche HTTP-Endpunkte aktivieren (z. B. Album-Löschung, "
            "Reset/DeviceName-Änderung)."
        ),
    )
    p.add_argument(
        "--pano-rows",
        type=int,
        default=4,
        help="Anzahl der Zeilen im Panorama-Raster (DWARF-Panorama-Modul)",
    )
    p.add_argument(
        "--pano-cols",
        type=int,
        default=4,
        help="Anzahl der Spalten im Panorama-Raster (DWARF-Panorama-Modul)",
    )
    p.add_argument(
        "--pano-grid-encoding",
        choices=["raw", "odd_index"],
        default="raw",
        help=(
            "Kodierung für pano rows/cols beim Setzen per cmd 16703. "
            "raw = Wert direkt senden; odd_index = value = count*2-1 (Debug/Test)."
        ),
    )
    p.add_argument(
        "--dump-db-all",
        action="store_true",
        help=(
            "SQLite device.db vollständig dumpen (Schema + alle Zeilen je Tabelle) "
            "in das JSON-Log. Kann sehr groß werden."
        ),
    )
    # FTP-Galerie-Check (anonym, gleiche Host-IP wie --host)
    p.add_argument(
        "--skip-ftp-gallery",
        action="store_true",
        help=(
            "FTP-Galerie-Check überspringen (keine Datei-Anzahlen der DWARF_II-Ordner "
            "im Log erfassen)."
        ),
    )
    p.add_argument(
        "--panorama-only",
        action="store_true",
        help="Nur Panorama-Kommandos ausführen (Motor/System überspringen).",
    )
    return p


def resolve_host(args_host: Optional[str]) -> str:
    host = args_host or os.environ.get("DWARF_HOST")
    if not host:
        raise SystemExit(
            "Kein Host angegeben. Nutze --host oder setze die Umgebungsvariable DWARF_HOST."
        )
    return host


def open_ws(host: str, timeout: float) -> WebSocket:
    url = f"ws://{host}:9900/"
    return create_connection(url, timeout=timeout)


WS_PACKET_MAJOR_VERSION = 1
WS_PACKET_MINOR_VERSION = 0
WS_PACKET_DEVICE_ID = 1
WS_PACKET_CLIENT_ID = "dwarf-capture"


def send_ws_command(
    ws: WebSocket,
    module_id: int,
    cmd: int,
    payload_builder: Optional[Callable[[], Optional[bytes]]],
    response_parser: Optional[Callable[[bytes], Any]],
    timeout: float,
    expected_module_id: Optional[int] = None,
    expected_cmd: Optional[int] = None,
) -> Dict[str, Any]:
    """Sendet ein WsPacket und wartet auf eine Antwort.

    Gibt eine Dictionary-Struktur mit Request/Response-Infos zurück.
    """
    req_info: Dict[str, Any] = {
        "module_id": module_id,
        "cmd": cmd,
    }

    # Payload erzeugen
    data_bytes: bytes = b""
    if payload_builder is not None:
        maybe_bytes = payload_builder()
        if maybe_bytes is not None:
            data_bytes = maybe_bytes

    if data_bytes:
        req_info["request_data_hex"] = data_bytes.hex()

    global WS_PACKET_MINOR_VERSION
    global WS_PACKET_DEVICE_ID

    # WsPacket erstellen
    packet = proto.WsPacket()
    packet.major_version = WS_PACKET_MAJOR_VERSION
    packet.minor_version = WS_PACKET_MINOR_VERSION
    packet.device_id = WS_PACKET_DEVICE_ID
    packet.module_id = module_id
    packet.cmd = cmd
    packet.type = 0  # Request
    packet.data = data_bytes
    packet.client_id = WS_PACKET_CLIENT_ID

    ws.settimeout(timeout)
    ws.send(packet.SerializeToString(), opcode=0x2)  # binary

    # Fire-and-forget mode: if there is no way to recognize a matching response,
    # do not block waiting. This is used for commands like panorama start where
    # progress notifications carry the real status.
    if response_parser is None and expected_module_id is None and expected_cmd is None:
        req_info["sent_only"] = True
        return req_info

    strict_expected = expected_module_id is not None or expected_cmd is not None
    exp_mod = expected_module_id if expected_module_id is not None else module_id
    exp_cmd = expected_cmd if expected_cmd is not None else cmd

    deadline = time.monotonic() + timeout
    last_raw: Optional[bytes] = None
    last_pkt: Optional[proto.WsPacket] = None

    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            req_info["error"] = "timeout_waiting_for_response"
            req_info["expected_response"] = {"module_id": exp_mod, "cmd": exp_cmd}
            if last_pkt is not None:
                req_info["last_seen_header"] = {
                    "major_version": last_pkt.major_version,
                    "minor_version": last_pkt.minor_version,
                    "device_id": last_pkt.device_id,
                    "module_id": last_pkt.module_id,
                    "cmd": last_pkt.cmd,
                    "type": last_pkt.type,
                }
                if last_pkt.data:
                    req_info["last_seen_data_hex"] = last_pkt.data.hex()
            if last_raw is not None:
                req_info["raw_hex"] = last_raw.hex()
            return req_info

        ws.settimeout(max(0.2, remaining))
        try:
            raw = ws.recv()
        except Exception as exc:  # noqa: BLE001
            req_info["error"] = f"recv_error: {exc!r}"
            return req_info

        if not isinstance(raw, (bytes, bytearray)):
            continue

        last_raw = bytes(raw)

        try:
            res_packet = proto.WsPacket()
            res_packet.ParseFromString(last_raw)
            last_pkt = res_packet
        except Exception as exc:  # noqa: BLE001
            req_info["error"] = f"parse_ws_packet_error: {exc!r}"
            req_info["raw_hex"] = last_raw.hex()
            return req_info

        if res_packet.type == 2 and (res_packet.module_id != exp_mod or res_packet.cmd != exp_cmd):
            continue

        WS_PACKET_MINOR_VERSION = res_packet.minor_version
        WS_PACKET_DEVICE_ID = res_packet.device_id

        parsed: Any = None
        if response_parser is not None and res_packet.data:
            try:
                parsed = response_parser(res_packet.data)
            except Exception as exc:  # noqa: BLE001
                req_info["response_parse_error"] = f"{exc!r}"

        if res_packet.module_id == exp_mod and res_packet.cmd == exp_cmd and res_packet.type != 0:
            break
        if response_parser is not None and parsed is not None and not strict_expected and res_packet.type != 0:
            break

    assert last_pkt is not None
    assert last_raw is not None
    res_packet = last_pkt

    req_info["response_header"] = {
        "major_version": res_packet.major_version,
        "minor_version": res_packet.minor_version,
        "device_id": res_packet.device_id,
        "module_id": res_packet.module_id,
        "cmd": res_packet.cmd,
        "type": res_packet.type,
    }
    req_info["expected_response"] = {"module_id": exp_mod, "cmd": exp_cmd}
    req_info["matched_expected"] = bool(
        res_packet.module_id == exp_mod and res_packet.cmd == exp_cmd
    )

    if response_parser is not None and res_packet.data:
        try:
            parsed = response_parser(res_packet.data)
            if hasattr(parsed, "code"):
                req_info["response_code"] = int(parsed.code)  # type: ignore[arg-type]
            req_info["response_payload"] = repr(parsed)
        except Exception as exc:  # noqa: BLE001
            req_info["response_parse_error"] = f"{exc!r}"
            req_info["response_data_hex"] = res_packet.data.hex()
    else:
        if res_packet.data:
            req_info["response_data_hex"] = res_packet.data.hex()

    return req_info


# Hilfsfunktionen zum Erzeugen von Payloads


def _pb_builder(message_cls: Any, **fields: Any) -> Callable[[], bytes]:
    def _build() -> bytes:
        msg = message_cls()
        for k, v in fields.items():
            setattr(msg, k, v)
        return msg.SerializeToString()

    return _build


def _empty_payload() -> Optional[bytes]:  # für leere message {}
    return b""


def _encode_varint(value: int) -> bytes:
    out = bytearray()
    v = int(value)
    while v > 0x7F:
        out.append((v & 0x7F) | 0x80)
        v >>= 7
    out.append(v & 0x7F)
    return bytes(out)


def _build_pano_grid_param_payload(selector_varint_bytes: bytes, value: int) -> bytes:
    payload = bytearray()
    payload.append(0x08)
    payload.extend(selector_varint_bytes)
    payload.append(0x10)
    payload.extend(_encode_varint(value))
    return bytes(payload)


def build_ws_command_definitions(
    include_dangerous_ws: bool,
    pano_rows: int,
    pano_cols: int,
    pano_grid_encoding: str,
    panorama_only: bool,
) -> List[Dict[str, Any]]:
    cmds: List[Dict[str, Any]] = []

    # Kamera Tele (MODULE_CAMERA_TELE)
    cmds.extend(
        [
            {
                "name": "tele_get_system_working_state",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10039,
                "payload_builder": _empty_payload,
                "response_parser": lambda b: _parse_pb(proto.ResSystemWorkingState, b),
                "dangerous": False,
            },
            {
                "name": "tele_open_camera",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10000,
                "payload_builder": _pb_builder(
                    proto.ReqOpenCamera,
                    binning=False,
                    rtsp_encode_type=0,
                ),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "tele_photo",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10002,
                "payload_builder": _pb_builder(proto.ReqPhoto),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "tele_burst",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10003,
                "payload_builder": _pb_builder(proto.ReqBurst, count=3),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "tele_start_record",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10005,
                "payload_builder": _pb_builder(proto.ReqStartRecord),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "tele_stop_record",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10006,
                "payload_builder": _pb_builder(proto.ReqStopRecord),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "tele_set_all_params",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10035,
                "payload_builder": _pb_builder(
                    proto.ReqSetAllParams,
                    exp_mode=0,
                    exp_index=0,
                    gain_mode=0,
                    gain_index=0,
                    ircut_value=0,
                    wb_mode=0,
                    wb_index_type=0,
                    wb_index=0,
                    brightness=0,
                    contrast=0,
                    hue=0,
                    saturation=0,
                    sharpness=50,
                    jpg_quality=80,
                ),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "tele_get_all_params",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10036,
                "payload_builder": _pb_builder(proto.ReqGetAllParams),
                "response_parser": lambda b: _parse_pb(proto.ResGetAllParams, b),
                "dangerous": False,
            },
            {
                "name": "tele_start_timelapse",
                "module_id": MODULE_CAMERA_TELE,
                "cmd": 10033,
                "payload_builder": _pb_builder(
                    proto.ReqStartTimelapsePhoto,
                    interval=1,
                    count=3,
                ),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
        ]
    )

    # Kamera Wide (MODULE_CAMERA_WIDE)
    cmds.extend(
        [
            {
                "name": "wide_open_camera",
                "module_id": MODULE_CAMERA_WIDE,
                "cmd": 12000,
                "payload_builder": _pb_builder(proto.ReqOpenCamera, binning=False),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "wide_photo",
                "module_id": MODULE_CAMERA_WIDE,
                "cmd": 12022,
                "payload_builder": _pb_builder(proto.ReqPhoto),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "wide_burst",
                "module_id": MODULE_CAMERA_WIDE,
                "cmd": 12023,
                "payload_builder": _pb_builder(proto.ReqBurst, count=3),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "wide_set_all_params",
                "module_id": MODULE_CAMERA_WIDE,
                "cmd": 12028,
                "payload_builder": _pb_builder(
                    proto.ReqSetAllParams,
                    exp_mode=0,
                    exp_index=0,
                    gain_mode=0,
                    gain_index=0,
                    ircut_value=0,
                    wb_mode=0,
                    wb_index_type=0,
                    wb_index=0,
                    brightness=0,
                    contrast=0,
                    hue=0,
                    saturation=0,
                    sharpness=50,
                    jpg_quality=80,
                ),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "wide_get_all_params",
                "module_id": MODULE_CAMERA_WIDE,
                "cmd": 12027,
                "payload_builder": _pb_builder(proto.ReqGetAllParams),
                "response_parser": lambda b: _parse_pb(proto.ResGetAllParams, b),
                "dangerous": False,
            },
        ]
    )

    # Fokus (MODULE_FOCUS)
    cmds.extend(
        [
            {
                "name": "focus_normal_auto",
                "module_id": MODULE_FOCUS,
                "cmd": 15000,
                "payload_builder": _pb_builder(
                    proto.ReqNormalAutoFocus,
                    mode=0,
                    center_x=0,
                    center_y=0,
                ),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "focus_astro_slow",
                "module_id": MODULE_FOCUS,
                "cmd": 15004,
                "payload_builder": _pb_builder(proto.ReqAstroAutoFocus, mode=0),
                "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "focus_astro_fast",
                "module_id": MODULE_FOCUS,
                "cmd": 15004,
                "payload_builder": _pb_builder(proto.ReqAstroAutoFocus, mode=1),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": True,
            },
            {
                "name": "focus_manual_single_step_far",
                "module_id": MODULE_FOCUS,
                "cmd": 15001,
                "payload_builder": _pb_builder(proto.ReqManualSingleStepFocus, direction=0),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": True,
            },
            {
                "name": "focus_manual_continuous_start_near",
                "module_id": MODULE_FOCUS,
                "cmd": 15002,
                "payload_builder": _pb_builder(proto.ReqStartManualContinuFocus, direction=1),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": True,
            },
            {
                "name": "focus_manual_continuous_stop",
                "module_id": MODULE_FOCUS,
                "cmd": 15003,
                "payload_builder": _pb_builder(proto.ReqStopManualContinuFocus),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": False,
            },
            {
                "name": "focus_astro_stop",
                "module_id": MODULE_FOCUS,
                "cmd": 15005,
                "payload_builder": _pb_builder(proto.ReqStopAstroAutoFocus),
                "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
                "dangerous": False,
            },
        ]
    )

    # Astro (MODULE_ASTRO)
    astro_cmds: List[Dict[str, Any]] = [
        {
            "name": "astro_start_calibration",
            "module_id": MODULE_ASTRO,
            "cmd": 11000,
            "payload_builder": _pb_builder(proto.ReqStartCalibration),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_stop_calibration",
            "module_id": MODULE_ASTRO,
            "cmd": 11001,
            "payload_builder": _pb_builder(proto.ReqStopCalibration),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_goto_dso",
            "module_id": MODULE_ASTRO,
            "cmd": 11002,
            "payload_builder": _pb_builder(
                proto.ReqGotoDSO,
                ra=0.0,
                dec=0.0,
                target_name="TEST_DSO",
            ),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_goto_solar_system",
            "module_id": MODULE_ASTRO,
            "cmd": 11003,
            "payload_builder": _pb_builder(
                proto.ReqGotoSolarSystem,
                index=9,
                lon=0.0,
                lat=0.0,
                target_name="Sun",
            ),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_stop_goto",
            "module_id": MODULE_ASTRO,
            "cmd": 11004,
            "payload_builder": _pb_builder(proto.ReqStopGoto),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_go_live",
            "module_id": MODULE_ASTRO,
            "cmd": 11010,
            "payload_builder": _pb_builder(proto.ReqGoLive),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": False,
        },
        {
            "name": "astro_start_eq_solving",
            "module_id": MODULE_ASTRO,
            "cmd": 11018,
            "payload_builder": _pb_builder(proto.ReqStartEqSolving, lon=0.0, lat=0.0),
            "response_parser": lambda b: _parse_pb(proto.ResStartEqSolving, b),
            "dangerous": True,
        },
        {
            "name": "astro_stop_eq_solving",
            "module_id": MODULE_ASTRO,
            "cmd": 11019,
            "payload_builder": _pb_builder(proto.ReqStopEqSolving),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_start_live_stacking_tele",
            "module_id": MODULE_ASTRO,
            "cmd": 11005,
            "payload_builder": _pb_builder(proto.ReqCaptureRawLiveStacking),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_stop_live_stacking_tele",
            "module_id": MODULE_ASTRO,
            "cmd": 11006,
            "payload_builder": _pb_builder(proto.ReqStopCaptureRawLiveStacking),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_start_live_stacking_wide",
            "module_id": MODULE_ASTRO,
            "cmd": 11016,
            "payload_builder": _pb_builder(proto.ReqCaptureWideRawLiveStacking),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_stop_live_stacking_wide",
            "module_id": MODULE_ASTRO,
            "cmd": 11017,
            "payload_builder": _pb_builder(proto.ReqStopCaptureWideRawLiveStacking),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_check_dark_frame",
            "module_id": MODULE_ASTRO,
            "cmd": 11009,
            "payload_builder": _pb_builder(proto.ReqCheckDarkFrame),
            "response_parser": lambda b: _parse_pb(proto.ResCheckDarkFrame, b),
            "dangerous": False,
        },
        {
            "name": "astro_capture_dark_frame",
            "module_id": MODULE_ASTRO,
            "cmd": 11007,
            "payload_builder": _pb_builder(proto.ReqCaptureDarkFrame, reshoot=0),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_stop_capture_dark_frame",
            "module_id": MODULE_ASTRO,
            "cmd": 11008,
            "payload_builder": _pb_builder(proto.ReqStopCaptureDarkFrame),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_capture_dark_frame_with_param",
            "module_id": MODULE_ASTRO,
            "cmd": 11021,
            "payload_builder": _pb_builder(
                proto.ReqCaptureDarkFrameWithParam,
                exp_index=0,
                gain_index=0,
                bin_index=0,
                cap_size=1,
            ),
            "response_parser": lambda b: _parse_pb(base_pb2.ComResponse, b),
            "dangerous": True,
        },
        {
            "name": "astro_get_dark_frame_list",
            "module_id": MODULE_ASTRO,
            "cmd": 11023,
            "payload_builder": _pb_builder(proto.ReqGetDarkFrameList),
            "response_parser": lambda b: _parse_pb(proto.ResGetDarkFrameInfoList, b),
            "dangerous": False,
        },
        {
            "name": "astro_delete_dark_frame_list",
            "module_id": MODULE_ASTRO,
            "cmd": 11024,
            "payload_builder": _pb_builder(proto.ReqDelDarkFrameList),
            "response_parser": lambda b: _parse_pb(proto.ResDelDarkFrameList, b),
            "dangerous": True,
        },
        {
            "name": "astro_delete_wide_dark_frame_list",
            "module_id": MODULE_ASTRO,
            "cmd": 11028,
            "payload_builder": _pb_builder(proto.ReqDelDarkFrameList),
            "response_parser": lambda b: _parse_pb(proto.ResDelDarkFrameList, b),
            "dangerous": True,
        },
        {
            "name": "astro_one_click_goto_dso",
            "module_id": MODULE_ASTRO,
            "cmd": 11013,
            "payload_builder": _pb_builder(
                proto.ReqOneClickGotoDSO,
                ra=0.0,
                dec=0.0,
                target_name="TEST_DSO",
            ),
            "response_parser": lambda b: _parse_pb(proto.ResOneClickGoto, b),
            "dangerous": False,
        },
        {
            "name": "astro_one_click_goto_solar",
            "module_id": MODULE_ASTRO,
            "cmd": 11014,
            "payload_builder": _pb_builder(
                proto.ReqOneClickGotoSolarSystem,
                index=9,
                lon=0.0,
                lat=0.0,
                target_name="Sun",
            ),
            "response_parser": lambda b: _parse_pb(proto.ResOneClickGoto, b),
            "dangerous": False,
        },
        {
            "name": "astro_stop_one_click_goto",
            "module_id": MODULE_ASTRO,
            "cmd": 11015,
            "payload_builder": _pb_builder(proto.ReqStopOneClickGoto),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
        },
    ]
    cmds.extend(astro_cmds)

    # Panorama (MODULE_PANORAMA)
    MODULE_NOTIFY_EXT = 15
    MODULE_PANORAMA_UI = 14
    CMD_PANO_UI_OPEN = 16402
    
    # Feature params for Panorama Grid (from dwarfium/dwarfii_api)
    # MODULE_CAMERA_TELE = 1
    CMD_CAMERA_TELE_SET_FEATURE_PARAM = 10037
    FEATURE_ID_PANO_ROW = 6
    FEATURE_ID_PANO_COL = 7

    pano_cmds: List[Dict[str, Any]] = [
        {
            "name": "panorama_ui_open",
            "module_id": MODULE_PANORAMA_UI,
            "cmd": CMD_PANO_UI_OPEN,
            "payload_builder": lambda: bytes.fromhex("0807"),
            "response_parser": None,
            "expected_module_id": MODULE_PANORAMA_UI,
            "expected_cmd": CMD_PANO_UI_OPEN,
            "dangerous": False,
        },
        {
            "name": "panorama_set_rows",
            "module_id": MODULE_CAMERA_TELE,
            "cmd": CMD_CAMERA_TELE_SET_FEATURE_PARAM,
            "payload_builder": _pb_builder(
                proto.ReqSetFeatureParams,
                param=proto.CommonParam(
                    id=FEATURE_ID_PANO_ROW,
                    mode_index=1,  # Continue Mode
                    continue_value=float(max(1, pano_rows)),
                ),
            ),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
            "log_response": True,
        },
        {
            "name": "panorama_set_cols",
            "module_id": MODULE_CAMERA_TELE,
            "cmd": CMD_CAMERA_TELE_SET_FEATURE_PARAM,
            "payload_builder": _pb_builder(
                proto.ReqSetFeatureParams,
                param=proto.CommonParam(
                    id=FEATURE_ID_PANO_COL,
                    mode_index=1,  # Continue Mode
                    continue_value=float(max(1, pano_cols)),
                ),
            ),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
            "log_response": True,
        },
        {
            "name": "panorama_start_grid",
            "module_id": MODULE_PANORAMA,
            "cmd": 15500,
            "payload_builder": _pb_builder(proto.ReqStartPanoramaByGrid),
            "response_parser": None,
            "dangerous": False,
            "pano_rows": pano_rows,
            "pano_cols": pano_cols,
        },
        # Hinweis: kein explizites panorama_stop-Kommando im Standardlauf,
        # damit das Panorama vollständig durchlaufen kann.
    ]
    cmds.extend(pano_cmds)

    if panorama_only:
        if not include_dangerous_ws:
            cmds = [c for c in cmds if not c.get("dangerous")]
        return cmds

    # Motor (MODULE_MOTOR)
    motor_cmds: List[Dict[str, Any]] = [
        {
            "name": "motor_goto_ra_1deg",
            "module_id": MODULE_MOTOR,
            "cmd": 14000,
            "payload_builder": _pb_builder(
                proto.ReqMotorRun,
                id=0,
                speed=1.0,
                direction=True,
                speed_ramping=100,
                resolution_level=0,
            ),
            "response_parser": lambda b: _parse_pb(proto.ResMotor, b),
            "dangerous": False,
        },
        {
            "name": "motor_stop_ra",
            "module_id": MODULE_MOTOR,
            "cmd": 14002,
            "payload_builder": _pb_builder(proto.ReqMotorStop, id=0),
            "response_parser": lambda b: _parse_pb(proto.ResMotor, b),
            "dangerous": False,
        },
        {
            "name": "motor_goto_dec_1deg",
            "module_id": MODULE_MOTOR,
            "cmd": 14000,
            "payload_builder": _pb_builder(
                proto.ReqMotorRun,
                id=1,
                speed=1.0,
                direction=True,
                speed_ramping=100,
                resolution_level=0,
            ),
            "response_parser": lambda b: _parse_pb(proto.ResMotor, b),
            "dangerous": False,
        },
        {
            "name": "motor_stop_dec",
            "module_id": MODULE_MOTOR,
            "cmd": 14002,
            "payload_builder": _pb_builder(proto.ReqMotorStop, id=1),
            "response_parser": lambda b: _parse_pb(proto.ResMotor, b),
            "dangerous": False,
        },
        {
            "name": "motor_joystick_small",
            "module_id": MODULE_MOTOR,
            "cmd": 14006,
            "payload_builder": _pb_builder(
                proto.ReqMotorServiceJoystick,
                vector_angle=0.0,
                vector_length=0.2,
                speed=1.0,
            ),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
        },
        {
            "name": "motor_joystick_stop",
            "module_id": MODULE_MOTOR,
            "cmd": 14008,
            "payload_builder": _pb_builder(proto.ReqMotorServiceJoystickStop),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
        },
        {
            "name": "dual_camera_linkage_center",
            "module_id": MODULE_MOTOR,
            "cmd": 14009,
            "payload_builder": _pb_builder(proto.ReqDualCameraLinkage, x=0, y=0),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
        },
    ]
    cmds.extend(motor_cmds)

    # System & RGB/Power (MODULE_SYSTEM + MODULE_RGB_POWER)
    system_cmds: List[Dict[str, Any]] = [
        {
            "name": "system_set_time",
            "module_id": MODULE_SYSTEM,
            "cmd": 13000,
            "payload_builder": _pb_builder(
                proto.ReqSetTime,
                timestamp=int(_dt.datetime.now(_dt.timezone.utc).timestamp()),
            ),
            "response_parser": lambda b: _parse_pb(proto.ComResponse, b),
            "dangerous": False,
        },
        {
            "name": "system_set_timezone_utc",
            "module_id": MODULE_SYSTEM,
            "cmd": 13001,
            "payload_builder": _pb_builder(proto.ReqSetTimezone, timezone="UTC"),
            "response_parser": None,  # Antwort ist Notification, nicht ComResponse
            "dangerous": False,
        },
        {
            "name": "system_set_mtp_mode_on",
            "module_id": MODULE_SYSTEM,
            "cmd": 13002,
            "payload_builder": _pb_builder(proto.ReqSetMtpMode, mode=1),
            "response_parser": None,
            "dangerous": False,
        },
        {
            "name": "system_set_cpu_mode_normal",
            "module_id": MODULE_SYSTEM,
            "cmd": 13003,
            "payload_builder": _pb_builder(proto.ReqSetCpuMode, mode=0),
            "response_parser": None,
            "dangerous": False,
        },
        {
            "name": "system_master_lock_off",
            "module_id": MODULE_SYSTEM,
            "cmd": 13004,  # Annahme: im System-CMD-Bereich; falls abweichend, liefert FW einen Fehlercode
            "payload_builder": _pb_builder(proto.ReqsetMasterLock, lock=False),
            "response_parser": None,
            "dangerous": False,
        },
        {
            "name": "rgb_open_ring",
            "module_id": MODULE_RGB_POWER,
            "cmd": 13500,
            "payload_builder": _pb_builder(proto.ReqOpenRgb),
            "response_parser": None,
            "dangerous": False,
        },
        {
            "name": "rgb_close_ring",
            "module_id": MODULE_RGB_POWER,
            "cmd": 13501,
            "payload_builder": _pb_builder(proto.ReqCloseRgb),
            "response_parser": None,
            "dangerous": False,
        },
        {
            "name": "power_indicator_on",
            "module_id": MODULE_RGB_POWER,
            "cmd": 13503,
            "payload_builder": _pb_builder(proto.ReqOpenPowerInd),
            "response_parser": None,
            "dangerous": False,
        },
        {
            "name": "power_indicator_off",
            "module_id": MODULE_RGB_POWER,
            "cmd": 13504,
            "payload_builder": _pb_builder(proto.ReqClosePowerInd),
            "response_parser": None,
            "dangerous": False,
        },
        ]
    cmds.extend(system_cmds)

    if not include_dangerous_ws:
        cmds = [c for c in cmds if not c.get("dangerous")]

    return cmds


def _parse_pb(message_cls: Any, data: bytes) -> Any:
    msg = message_cls()
    msg.ParseFromString(data)
    return msg


# HTTP-Endpunkte


def build_http_endpoints(include_dangerous_http: bool) -> List[Dict[str, Any]]:
    eps: List[Dict[str, Any]] = []

    def ep(
        name: str,
        method: str,
        path: str,
        *,
        port: int = 8082,
        json_body: Optional[Dict[str, Any]] = None,
        stream: bool = False,
        dangerous: bool = False,
    ) -> None:
        eps.append(
            {
                "name": name,
                "method": method,
                "path": path,
                "port": port,
                "json_body": json_body,
                "stream": stream,
                "dangerous": dangerous,
            }
        )

    # Album / Firmware / Device / Log, nach Doku
    ep("album_media_counts", "POST", "/album/list/mediaCounts", json_body={})
    ep(
        "album_media_infos",
        "POST",
        "/album/list/mediaInfos",
        json_body={"mediaType": 0, "pageIndex": 0, "pageSize": 0},
    )
    ep(
        "album_delete",
        "POST",
        "/album/delete",
        json_body={"datas": []},  # leere Liste => sollte nichts löschen
        dangerous=True,
    )

    # Konfiguration / Firmware-Version
    ep("firmware_version", "POST", "/firmwareVersion", json_body={})
    ep("get_default_params_config", "GET", "/getDefaultParamsConfig")

    # Logs
    ep("log_info", "GET", "/logInfo")
    ep("download_log", "GET", "/downloadLog")

    # Device-Info & Reset
    ep("device_info", "POST", "/deviceInfo", json_body={})
    ep(
        "set_device_name_and_psd",
        "POST",
        "/setDeviceNameAndPsd",
        json_body={"mode": 0, "oldValue": "", "newValue": ""},
        dangerous=True,
    )
    ep("reset_device_info", "POST", "/resetDeviceInfo", json_body={}, dangerous=True)
    ep("get_reset_state", "POST", "/getResetState", json_body={})

    # JPG-Streams
    ep("tele_jpg_stream", "GET", "/mainstream", port=8092, stream=True)
    ep("wide_jpg_stream", "GET", "/secondstream", port=8092, stream=True)

    if not include_dangerous_http:
        eps = [e for e in eps if not e.get("dangerous")]

    return eps


def http_health_check(host: str, timeout: float) -> Dict[str, Any]:
    """Kleiner Health-Check gegen /deviceInfo.

    Gibt ein Dict mit ok/status/error zurück und loggt nichts.
    """
    url = f"http://{host}:8082/deviceInfo"
    try:
        resp = requests.post(url, json={}, timeout=min(timeout, 3.0))
        return {"ok": bool(resp.ok), "status": resp.status_code}
    except Exception as exc:  # noqa: BLE001
        return {"ok": False, "error": repr(exc)}


def call_http_endpoints(
    host: str,
    endpoints: List[Dict[str, Any]],
    timeout: float,
) -> List[Dict[str, Any]]:
    results: List[Dict[str, Any]] = []

    sess = requests.Session()

    for ep in endpoints:
        entry: Dict[str, Any] = {
            "name": ep["name"],
            "method": ep["method"],
            "path": ep["path"],
            "port": ep["port"],
        }
        url = f"http://{host}:{ep['port']}{ep['path']}"
        entry["url"] = url
        # Logging auf STDOUT: geplanter HTTP-Aufruf
        print(f"[HTTP] {ep['method']} {url}")
        try:
            if ep["method"] == "GET":
                resp = sess.get(url, timeout=timeout, stream=ep.get("stream", False))
            else:
                resp = sess.post(
                    url,
                    json=ep.get("json_body"),
                    timeout=timeout,
                    stream=ep.get("stream", False),
                )
            entry["status_code"] = resp.status_code
            print(f"[HTTP] {ep['name']} -> status={resp.status_code}")
            entry["headers"] = dict(list(resp.headers.items())[:20])

            if ep.get("stream"):
                # Nur den ersten Chunk lesen, um nicht unendlich zu streamen
                try:
                    chunk = next(resp.iter_content(chunk_size=8192))
                    entry["first_chunk_len"] = len(chunk)
                except StopIteration:
                    entry["first_chunk_len"] = 0
            else:
                ct = resp.headers.get("Content-Type", "")
                text = resp.text
                if "application/json" in ct:
                    try:
                        entry["json_body"] = resp.json()
                    except Exception as exc:  # noqa: BLE001
                        entry["body_parse_error"] = f"{exc!r}"
                        entry["text_body_snippet"] = text[:1000]
                else:
                    entry["text_body_snippet"] = text[:1000]
        except Exception as exc:  # noqa: BLE001
            entry["error"] = f"request_error: {exc!r}"

        results.append(entry)
    return results


# Panorama-Abschluss erkennen

PANORAMA_NOTIFY_CMD_PROGRESS = 15219  # CMD_NOTIFY_PANORAMA_PROGRESS
MODULE_NOTIFY_ID = MODULE_NOTIFY


def wait_for_panorama_completion(
    ws: WebSocket,
    *,
    max_wait_sec: float = 600.0,
    idle_timeout: float = 5.0,
) -> Dict[str, Any]:
    """Wartet auf das Ende der Panorama-Aufnahme.

    Nutzt CMD_NOTIFY_PANORAMA_PROGRESS (15219) und wertet
    ResNotifyPanoramaProgress (total_count/completed_count) aus.
    """
    start = time.monotonic()
    events: List[Dict[str, Any]] = []

    other_notify_events = 0

    last_ping = time.monotonic()

    while True:
        elapsed = time.monotonic() - start
        if elapsed >= max_wait_sec:
            return {
                "status": "timeout",
                "duration_sec": round(elapsed, 1),
                "events": events,
            }

        # Keepalive: some firmwares close the WS connection during long panorama runs
        # if the client does not send pings.
        if time.monotonic() - last_ping >= 10.0:
            try:
                ws.ping()
            except Exception:
                pass
            last_ping = time.monotonic()

        remaining = max_wait_sec - elapsed
        ws.settimeout(min(idle_timeout, max(1.0, remaining)))
        try:
            raw = ws.recv()
        except Exception as exc:  # noqa: BLE001
            return {
                "status": "recv_error",
                "error": repr(exc),
                "duration_sec": round(time.monotonic() - start, 1),
                "events": events,
            }

        if not isinstance(raw, (bytes, bytearray)):
            continue

        try:
            pkt = proto.WsPacket()
            pkt.ParseFromString(raw)
        except Exception:
            continue

        if pkt.module_id == MODULE_NOTIFY_ID and pkt.cmd != PANORAMA_NOTIFY_CMD_PROGRESS:
            if other_notify_events < 50:
                events.append(
                    {
                        "t": round(time.monotonic() - start, 2),
                        "type": "other_notify",
                        "cmd": int(pkt.cmd),
                        "data_hex": pkt.data.hex() if pkt.data else "",
                    }
                )
                other_notify_events += 1
            continue

        if pkt.module_id != MODULE_NOTIFY_ID or pkt.cmd != PANORAMA_NOTIFY_CMD_PROGRESS:
            continue

        try:
            prog = proto.ResNotifyPanoramaProgress()
            prog.ParseFromString(pkt.data)
            ev = {
                "total_count": int(prog.total_count),
                "completed_count": int(prog.completed_count),
            }
            events.append(ev)
            print(
                f"[WS] Panorama-Progress: {prog.completed_count}/{prog.total_count} Aufnahmen"
            )
            if prog.total_count > 0 and prog.completed_count >= prog.total_count:
                return {
                    "status": "completed",
                    "duration_sec": round(time.monotonic() - start, 1),
                    "events": events,
                }
        except Exception:
            # Falls das Parsing fehlschlägt, weiter lauschen
            continue


# FTP-Galerie-Check

def _ftp_connect_anonymous(host: str, timeout: float) -> FTP:
    ftp = FTP()
    ftp.connect(host=host, timeout=timeout)
    ftp.login(user="anonymous", passwd="")
    return ftp


def _ftp_count_files_in_dir(ftp: FTP, directory: str) -> Optional[int]:
    """Zähle Galerie-Einträge je nach Typ des Verzeichnisses.

    - Normal_Photos / Videos: Anzahl der Dateien im Verzeichnis (keine Rekursion).
    - data: Anzahl der Dateien im Verzeichnis (DB-Analyse erfolgt separat).
    - Astronomy / Burst / Panoramas: Anzahl der Galerie-Einträge über
      *thumbnail*-Dateien im Verzeichnis und in direkten Unterordnern.
    """
    try:
        # Ausgangsverzeichnis merken, damit wir am Ende zurückkehren können
        orig_cwd = ftp.pwd()
    except Exception:
        orig_cwd = None

    lower_dir = directory.lower().rstrip("/")

    # Spezialfälle: Normal_Photos und Videos -> echte Dateien zählen
    if lower_dir.endswith("/normal_photos") or lower_dir.endswith("/videos") or lower_dir.endswith("/data"):
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
            try:
                names = ftp.nlst()
            except error_perm:
                return None
            for name in names:
                # Heuristik: Einträge ohne Punkt sind eher Verzeichnisse
                if "." in name:
                    count += 1
        if orig_cwd is not None:
            try:
                ftp.cwd(orig_cwd)
            except error_perm:
                pass
        return count

    # Standard: *thumbnail*-Zählung (Astronomy/Burst/Panoramas)
    # Verzeichnis-spezifische Thumbnail-Namen:
    # - Astronomy:  stacked_thumbnail
    # - Burst:      burst_thumbnail
    # - Panoramas:  panorama_thumbnail
    last_seg = lower_dir.rsplit("/", 1)[-1]
    if last_seg == "astronomy":
        thumb_kw = "stacked_thumbnail"
    elif last_seg == "burst":
        thumb_kw = "burst_thumbnail"
    elif last_seg == "panoramas":
        thumb_kw = "panorama_thumbnail"
    else:
        thumb_kw = "thumbnail"

    try:
        ftp.cwd(directory)
    except error_perm:
        return None

    count = 0
    subdirs = []

    # Erstes Level: Dateien + Unterordner einsammeln
    try:
        for facts, name in ftp.mlsd():
            ftype = facts.get("type")
            lower_name = name.lower()
            if ftype == "file":
                if thumb_kw in lower_name:
                    count += 1
            elif ftype == "dir":
                subdirs.append(name)
    except (error_perm, AttributeError):
        # Fallback ohne MLSD: wir kennen Typen nicht, versuchen Heuristik
        try:
            names = ftp.nlst()
        except error_perm:
            return None
        for name in names:
            lower_name = name.lower()
            # Alles mit passendem Thumbnail-Muster zählen
            if thumb_kw in lower_name:
                count += 1

    # Zweites Level: direkte Unterordner nach *thumbnail* durchsuchen
    for d in subdirs:
        try:
            ftp.cwd(d)
        except error_perm:
            continue
        try:
            for facts, name in ftp.mlsd():
                if facts.get("type") == "file" and thumb_kw in name.lower():
                    count += 1
        except (error_perm, AttributeError):
            try:
                names = ftp.nlst()
            except error_perm:
                names = []
            for name in names:
                if thumb_kw in name.lower():
                    count += 1
        # zurück ins übergeordnete Verzeichnis
        try:
            ftp.cwd("..")
        except error_perm:
            pass

    # Ursprüngliches Verzeichnis wiederherstellen
    if orig_cwd is not None:
        try:
            ftp.cwd(orig_cwd)
        except error_perm:
            pass

    return count


def _jsonify_sql_value(v: Any) -> Any:
    if v is None:
        return None
    if isinstance(v, (int, float, str, bool)):
        return v
    if isinstance(v, (bytes, bytearray, memoryview)):
        return {"__bytes_hex__": bytes(v).hex()}
    return repr(v)


def _ftp_analyze_data_db(
    ftp: FTP,
    base_dir: str,
    local_base: str,
    dump_all: bool,
) -> Dict[str, Any]:
    """Lade Datenbank(en) aus /DWARF_II/data herunter und werte sie grob aus.

    - Sucht nach Dateien mit Endungen .db / .sqlite / .sqlite3 (typisch: device.db).
    - Ignoriert SQLite-Interna wie device.db-shm / device.db-wal.
    - Lädt die Dateien nach local_base.
    - Versucht, sie als SQLite zu öffnen und listet Tabellen + Zeilenzahl.
    """
    info: Dict[str, Any] = {"ok": False, "error": None, "files": []}
    try:
        ftp.cwd(base_dir)
    except error_perm as exc:
        info["error"] = f"ftp_cwd_error: {exc!r}"
        return info

    try:
        names = [n for n in ftp.nlst() if "." in n]
    except error_perm as exc:
        info["error"] = f"ftp_list_error: {exc!r}"
        return info

    os.makedirs(local_base, exist_ok=True)

    for name in names:
        lower = name.lower()
        if not (lower.endswith(".db") or lower.endswith(".sqlite") or lower.endswith(".sqlite3")):
            continue
        local_path = os.path.join(local_base, name)
        try:
            with open(local_path, "wb") as f:
                ftp.retrbinary(f"RETR {name}", f.write)
        except Exception as exc:  # noqa: BLE001
            info["files"].append(
                {
                    "name": name,
                    "local_path": local_path,
                    "download_error": repr(exc),
                }
            )
            continue

        file_entry: Dict[str, Any] = {
            "name": name,
            "local_path": local_path,
        }
        # SQLite-Analyse (best effort)
        try:
            conn = sqlite3.connect(local_path)
            try:
                cur = conn.cursor()
                cur.execute(
                    "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"
                )
                tables = []
                for (tname,) in cur.fetchall():
                    table_entry: Dict[str, Any] = {"name": tname, "row_count": None}
                    try:
                        cur.execute(f"SELECT COUNT(*) FROM '{tname}'")
                        (cnt,) = cur.fetchone() or (None,)
                        table_entry["row_count"] = cnt
                    except Exception:
                        table_entry["row_count"] = None

                    if dump_all:
                        try:
                            cur.execute(f"PRAGMA table_info('{tname}')")
                            cols = []
                            for cid, name_, ctype, notnull, dflt, pk in cur.fetchall():
                                cols.append(
                                    {
                                        "cid": cid,
                                        "name": name_,
                                        "type": ctype,
                                        "notnull": notnull,
                                        "default": dflt,
                                        "pk": pk,
                                    }
                                )
                            table_entry["columns"] = cols
                        except Exception as exc:  # noqa: BLE001
                            table_entry["columns_error"] = repr(exc)

                        try:
                            cur.execute(f"SELECT * FROM '{tname}'")
                            rows = []
                            for row in cur.fetchall():
                                rows.append([_jsonify_sql_value(v) for v in row])
                            table_entry["rows"] = rows
                        except Exception as exc:  # noqa: BLE001
                            table_entry["rows_error"] = repr(exc)

                    tables.append(table_entry)

                file_entry["tables"] = tables
            finally:
                conn.close()
        except Exception as exc:  # noqa: BLE001
            file_entry["sqlite_error"] = repr(exc)

        info["files"].append(file_entry)

    info["ok"] = True
    return info


def ftp_gallery_overview(host: str, timeout: float, dump_db_all: bool) -> Dict[str, Any]:
    """Liefert Datei-Anzahlen je Galerie-Verzeichnis über FTP.

    Struktur:
        {
          "ok": bool,
          "error": str | None,
          "dirs": { "<pfad>": anzahl | null, ... },
          "data_db": { ... } | null
        }
    """
    result: Dict[str, Any] = {"ok": False, "error": None, "dirs": {}, "data_db": None}
    ftp: Optional[FTP] = None
    try:
        ftp = _ftp_connect_anonymous(host, timeout)
        for d in GALLERY_DIRS:
            if d.endswith("/data"):
                cnt = _ftp_count_files_in_dir(ftp, d)
                result["dirs"][d] = cnt
                # Datenbank-Analyse (typischerweise device.db)
                local_base = os.path.join(REPO_ROOT, "capture", "data_downloads")
                result["data_db"] = _ftp_analyze_data_db(ftp, d, local_base, dump_db_all)
            else:
                cnt = _ftp_count_files_in_dir(ftp, d)
                result["dirs"][d] = cnt
        result["ok"] = True
    except Exception as exc:  # noqa: BLE001
        result["error"] = repr(exc)
    finally:
        if ftp is not None:
            try:
                ftp.quit()
            except Exception:
                try:
                    ftp.close()
                except Exception:
                    pass

    return result


def main() -> None:
    parser = build_arg_parser()
    args = parser.parse_args()

    host = resolve_host(args.host)

    capture_dir = os.path.join(REPO_ROOT, "capture")
    os.makedirs(capture_dir, exist_ok=True)

    now_utc = _dt.datetime.now(_dt.timezone.utc)
    timestamp = now_utc.strftime("%Y%m%d_%H%M%S")
    log_path = os.path.join(capture_dir, f"dwarf_capture_{timestamp}.json")

    # HTTP-Health-Check vorab, damit klar ist, ob Port 8082 überhaupt erreichbar ist.
    http_health = http_health_check(host, args.timeout)
    if not http_health.get("ok"):
        print(f"[HTTP] Health-Check /deviceInfo FEHLER: {http_health}")
    else:
        print(f"[HTTP] Health-Check /deviceInfo OK: status={http_health.get('status')}")

    log_root: Dict[str, Any] = {
        "meta": {
            "host": host,
            "started_at": _now_iso(),
            "http_health": http_health,
        },
        "ws_results": [],
        "http_results": [],
    }

    # WebSocket-Teil
    if not args.skip_ws:
        try:
            ws = open_ws(host, args.timeout)
        except Exception as exc:  # noqa: BLE001
            log_root.setdefault("errors", []).append(
                {"phase": "ws_connect", "error": f"{exc!r}"}
            )
            ws = None

        if ws is not None:
            try:
                ws_cmds = build_ws_command_definitions(
                    args.include_dangerous_ws,
                    pano_rows=max(1, args.pano_rows),
                    pano_cols=max(1, args.pano_cols),
                    pano_grid_encoding=args.pano_grid_encoding,
                    panorama_only=args.panorama_only,
                )
                # Panorama-Parameter im Log festhalten
                log_root.setdefault("meta", {})["panorama_params"] = {
                    "rows": max(1, args.pano_rows),
                    "cols": max(1, args.pano_cols),
                    "grid_encoding": args.pano_grid_encoding,
                }

                ws_broken = False

                for c in ws_cmds:
                    if ws_broken:
                        break
                    # Logging auf STDOUT: geplanter WS-Befehl
                    print(
                        f"[WS] {c['name']} (module={c['module_id']}, cmd={c['cmd']})"
                    )
                    try:
                        res = send_ws_command(
                            ws=ws,
                            module_id=c["module_id"],
                            cmd=c["cmd"],
                            payload_builder=c["payload_builder"],
                            response_parser=c["response_parser"],
                            timeout=args.timeout,
                            expected_module_id=c.get("expected_module_id"),
                            expected_cmd=c.get("expected_cmd"),
                        )
                        res["name"] = c["name"]
                        res["dangerous"] = bool(c.get("dangerous"))
                        log_root["ws_results"].append(res)
                        # Ergebnis auf STDOUT
                        print(
                            f"[WS] {c['name']} -> code={res.get('response_code')} "
                            f"error={res.get('error')}"
                        )
                        
                        # Detailliertes Logging für Feature Param Commands
                        if c.get('log_response') and res.get('parsed_response'):
                            parsed = res['parsed_response']
                            print(f"[WS]   Response details: {parsed}")

                        # Speziell für Panorama: auf Abschluss der Hintergrund-Aufnahme warten
                        if c["name"] == "panorama_start_grid":
                            # rows/cols aus dem Command-Def extrahieren (falls gesetzt)
                            pano_rows = c.get("pano_rows")
                            pano_cols = c.get("pano_cols")
                            print(
                                f"[WS] Warte auf Ende der Panorama-Aufnahme (Notify 15219)... "
                                f"(rows={pano_rows}, cols={pano_cols})"
                            )
                            pano_info = wait_for_panorama_completion(ws)
                            log_root.setdefault("meta", {})["panorama_wait"] = pano_info
                            print(f"[WS] Panorama-Ende: {pano_info}")
                            if pano_info.get("status") == "recv_error":
                                try:
                                    ws2 = open_ws(host, args.timeout)
                                    try:
                                        pano_info2 = wait_for_panorama_completion(ws2)
                                        log_root.setdefault("meta", {})[
                                            "panorama_wait_reconnect"
                                        ] = pano_info2
                                        print(f"[WS] Panorama-Ende (Reconnect): {pano_info2}")
                                        pano_info = pano_info2
                                    finally:
                                        try:
                                            ws2.close(status=1000, reason="dwarf_capture_done")
                                        except Exception:
                                            pass
                                except Exception as exc:  # noqa: BLE001
                                    log_root.setdefault("meta", {})[
                                        "panorama_wait_reconnect"
                                    ] = {"status": "connect_error", "error": repr(exc)}
                            # Wenn die Verbindung verloren geht oder Timeout: keine weiteren WS-Kommandos senden
                            if pano_info.get("status") != "completed":
                                ws_broken = True
                    except Exception as exc:  # noqa: BLE001
                        err = f"send_ws_command_error: {exc!r}"
                        log_root["ws_results"].append(
                            {
                                "name": c["name"],
                                "module_id": c["module_id"],
                                "cmd": c["cmd"],
                                "dangerous": bool(c.get("dangerous")),
                                "error": err,
                            }
                        )
                        print(f"[WS] {c['name']} -> ERROR {err}")

                    # Für Motor- und Panorama-Kommandos: definierte Laufzeit von 5 s.
                    # Die eigentlichen Stop-Kommandos werden anschließend in der
                    # gleichen Schleife ausgeführt.
                    if c["name"] in (
                        "motor_goto_ra_1deg",
                        "motor_goto_dec_1deg",
                        "motor_joystick_small",
                    ):
                        time.sleep(5.0)

                # Am Ende sicherstellen, dass Ringlicht und Power-Indikator eingeschaltet sind.
                if not ws_broken:
                    for final_cmd in (
                        {
                            "name": "rgb_open_ring_final",
                            "module_id": MODULE_RGB_POWER,
                            "cmd": 13500,
                            "builder": _pb_builder(proto.ReqOpenRgb),
                        },
                        {
                            "name": "power_indicator_on_final",
                            "module_id": MODULE_RGB_POWER,
                            "cmd": 13503,
                            "builder": _pb_builder(proto.ReqOpenPowerInd),
                        },
                    ):
                        print(
                            f"[WS] {final_cmd['name']} (module={final_cmd['module_id']}, cmd={final_cmd['cmd']})"
                        )
                    try:
                        res = send_ws_command(
                            ws=ws,
                            module_id=final_cmd["module_id"],
                            cmd=final_cmd["cmd"],
                            payload_builder=final_cmd["builder"],
                            response_parser=lambda b: _parse_pb(proto.ComResponse, b),
                            timeout=args.timeout,
                        )
                        res["name"] = final_cmd["name"]
                        res["dangerous"] = False
                        log_root["ws_results"].append(res)
                        print(
                            f"[WS] {final_cmd['name']} -> code={res.get('response_code')} "
                            f"error={res.get('error')}"
                        )
                    except Exception as exc:  # noqa: BLE001
                        err = f"send_ws_command_error: {exc!r}"
                        log_root["ws_results"].append(
                            {
                                "name": final_cmd["name"],
                                "module_id": final_cmd["module_id"],
                                "cmd": final_cmd["cmd"],
                                "dangerous": False,
                                "error": err,
                            }
                        )
                        print(f"[WS] {final_cmd['name']} -> ERROR {err}")
            finally:
                try:
                    print("[WS] Verbindung schließen (Close-Frame senden)")
                    # Versuche einen sauberen WebSocket-Disconnect mit Close-Frame.
                    ws.close(status=1000, reason="dwarf_capture_done")
                except Exception:
                    pass

    # HTTP-Teil
    if not args.skip_http:
        http_eps = build_http_endpoints(args.include_dangerous_http)
        http_results = call_http_endpoints(host, http_eps, args.timeout)
        log_root["http_results"] = http_results

    # FTP-Galerie-Check (optional, Standard: aktiv)
    if not args.skip_ftp_gallery:
        print("[FTP] Galerie-Check starten (anonymous@DWARF_II)")
        ftp_info = ftp_gallery_overview(host, args.timeout, bool(args.dump_db_all))
        log_root["meta"]["ftp_gallery"] = ftp_info
        if not ftp_info.get("ok"):
            print(f"[FTP] Galerie-Check FEHLER: {ftp_info.get('error')}")
        else:
            for d, cnt in ftp_info.get("dirs", {}).items():
                print(f"[FTP] {d}: {cnt if cnt is not None else 'NICHT ERREICHBAR'} Dateien")

    log_root["meta"]["finished_at"] = _now_iso()

    with open(log_path, "w", encoding="utf-8") as f:
        json.dump(log_root, f, indent=2, ensure_ascii=False)

    print(f"Log geschrieben nach: {log_path}")


if __name__ == "__main__":
    main()
