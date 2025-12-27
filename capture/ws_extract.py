#!/usr/bin/env python3
import argparse
import subprocess
import sys
from dataclasses import dataclass
from typing import Iterable, List, Optional, Tuple


@dataclass(frozen=True)
class WsBinaryCommand:
    pcap: str
    frame_number: int
    time_relative: float
    tcp_len: int
    ws_opcode: int
    payload: bytes


def run_tshark_extract(
    pcap_path: str,
    dst_port: int,
    host: Optional[str],
    direction: str,
) -> str:
    # Extract WebSocket-over-TCP payload bytes (hex-encoded in -e data).
    #
    # c2s: client -> server, tcp.dstport == dst_port
    # s2c: server -> client, tcp.srcport == dst_port
    if direction == "c2s":
        display_filter = f"tcp.dstport=={dst_port} && tcp.len>0"
    elif direction == "s2c":
        display_filter = f"tcp.srcport=={dst_port} && tcp.len>0"
    else:
        raise ValueError(f"unsupported direction: {direction}")
    if host:
        display_filter = f"({display_filter}) && ip.addr=={host}"

    cmd = [
        "tshark",
        "-r",
        pcap_path,
        "-Y",
        display_filter,
        "-T",
        "fields",
        "-E",
        "separator=\t",
        "-e",
        "frame.number",
        "-e",
        "frame.time_relative",
        "-e",
        "tcp.len",
        "-e",
        "data",
    ]

    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        raise RuntimeError(
            "tshark failed. Command:\n"
            + " ".join(cmd)
            + "\n\nstdout:\n"
            + (proc.stdout or "")
            + "\n\nstderr:\n"
            + (proc.stderr or "")
        )

    return proc.stdout


def unmask_ws_frame(hexstr: str) -> Tuple[int, bytes]:
    b = bytes.fromhex(hexstr)
    if len(b) < 2:
        raise ValueError("WS frame too short")

    opcode = b[0]
    b1 = b[1]

    masked = (b1 & 0x80) != 0
    ln = b1 & 0x7F
    i = 2

    if ln == 126:
        if len(b) < i + 2:
            raise ValueError("WS frame truncated (len=126)")
        ln = int.from_bytes(b[i : i + 2], "big")
        i += 2
    elif ln == 127:
        if len(b) < i + 8:
            raise ValueError("WS frame truncated (len=127)")
        ln = int.from_bytes(b[i : i + 8], "big")
        i += 8

    if masked:
        if len(b) < i + 4:
            raise ValueError("WS frame truncated (mask)")
        mask = b[i : i + 4]
        i += 4
        if len(b) < i + ln:
            raise ValueError("WS frame truncated (payload)")
        payload = bytearray(b[i : i + ln])
        for k in range(ln):
            payload[k] ^= mask[k % 4]
        return opcode, bytes(payload)

    if len(b) < i + ln:
        raise ValueError("WS frame truncated (unmasked payload)")
    return opcode, b[i : i + ln]


def parse_commands_from_tshark(pcap: str, tshark_out: str) -> List[WsBinaryCommand]:
    out: List[WsBinaryCommand] = []
    for raw_line in tshark_out.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        parts = line.split("\t")
        if len(parts) < 4:
            continue

        frame_s, t_s, tcp_len_s, hexdata = parts[0], parts[1], parts[2], parts[3]
        try:
            frame_number = int(frame_s)
            time_relative = float(t_s)
            tcp_len = int(tcp_len_s)
            ws_opcode, payload = unmask_ws_frame(hexdata)
        except Exception:
            # Ignore malformed frames
            continue

        # 0x82 == WS binary
        if ws_opcode != 0x82:
            continue

        out.append(
            WsBinaryCommand(
                pcap=pcap,
                frame_number=frame_number,
                time_relative=time_relative,
                tcp_len=tcp_len,
                ws_opcode=ws_opcode,
                payload=payload,
            )
        )

    out.sort(key=lambda x: x.frame_number)
    return out


def diff_bytes(a: bytes, b: bytes) -> List[Tuple[int, int, int]]:
    n = min(len(a), len(b))
    diffs: List[Tuple[int, int, int]] = []
    for i in range(n):
        if a[i] != b[i]:
            diffs.append((i, a[i], b[i]))
    # length diffs are reported separately by caller
    return diffs


def print_summary(cmds: List[WsBinaryCommand], show_payload: bool) -> None:
    print(f"Binary commands: {len(cmds)}")
    for c in cmds:
        head = c.payload[:12].hex()
        print(
            f"- frame {c.frame_number:>4} t={c.time_relative:>8.3f}s payload_len={len(c.payload)} head={head}"
        )
        if show_payload:
            print(c.payload.hex())


def print_consecutive_diffs(cmds: List[WsBinaryCommand], max_changes: int) -> None:
    print("\nDiffs between consecutive binary commands:")
    if len(cmds) < 2:
        return

    for prev, cur in zip(cmds, cmds[1:]):
        diffs = diff_bytes(prev.payload, cur.payload)
        length_change = len(prev.payload) != len(cur.payload)
        if length_change or (diffs and len(diffs) <= max_changes):
            print(
                f"{prev.frame_number}->{cur.frame_number} changes={len(diffs)}",
                end="  ",
            )
            if length_change:
                print(f"[len:{len(prev.payload)}->{len(cur.payload)}]", end=" ")
            for idx, x, y in diffs:
                print(f"[{idx}:{x:02x}->{y:02x}]", end=" ")
            print()


def print_diff_between(
    a: List[WsBinaryCommand],
    b: List[WsBinaryCommand],
    max_changes: int,
) -> None:
    print("\nDiff between first binary command of each capture:")
    if not a or not b:
        return

    pa = a[0].payload
    pb = b[0].payload

    diffs = diff_bytes(pa, pb)
    length_change = len(pa) != len(pb)

    print(f"{a[0].pcap}:{a[0].frame_number}  vs  {b[0].pcap}:{b[0].frame_number}")
    print(f"payload_len {len(pa)} vs {len(pb)}")

    if length_change:
        print(f"[len:{len(pa)}->{len(pb)}]")
    for idx, x, y in diffs[:max_changes]:
        print(f"[{idx}:{x:02x}->{y:02x}]")
    if len(diffs) > max_changes:
        print(f"... ({len(diffs) - max_changes} more changes)")


def main(argv: Optional[List[str]] = None) -> int:
    p = argparse.ArgumentParser(
        description="Extract and diff DWARF WebSocket binary commands from pcapng (client->server dstport=9900)."
    )
    p.add_argument("pcaps", nargs="+", help="pcap/pcapng files")
    p.add_argument("--dst-port", type=int, default=9900, help="destination TCP port (default: 9900)")
    p.add_argument("--host", default=None, help="optional IP filter (e.g. 10.42.0.209)")
    p.add_argument(
        "--direction",
        choices=["c2s", "s2c", "both"],
        default="c2s",
        help="traffic direction to extract (default: c2s). 'both' prints two sections per pcap.",
    )
    p.add_argument("--show-payload", action="store_true", help="print full unmasked payload hex")
    p.add_argument("--no-consecutive-diff", action="store_true", help="disable diffing consecutive commands")
    p.add_argument(
        "--diff-first-two",
        action="store_true",
        help="if two pcaps are provided, also diff first binary command between pcap[0] and pcap[1]",
    )
    p.add_argument("--max-changes", type=int, default=16, help="max byte-changes to show in diffs")

    args = p.parse_args(argv)

    def process_one(pcap: str, direction: str) -> List[WsBinaryCommand]:
        try:
            tshark_out = run_tshark_extract(pcap, args.dst_port, args.host, direction)
        except Exception as e:
            print(str(e), file=sys.stderr)
            raise
        return parse_commands_from_tshark(pcap, tshark_out)

    all_cmds: List[List[WsBinaryCommand]] = []
    for pcap in args.pcaps:
        if args.direction in ("c2s", "s2c"):
            try:
                cmds = process_one(pcap, args.direction)
            except Exception:
                return 2
            all_cmds.append(cmds)

            print(f"\n== {pcap} ({args.direction}) ==")
            print_summary(cmds, show_payload=args.show_payload)
            if not args.no_consecutive_diff:
                print_consecutive_diffs(cmds, max_changes=args.max_changes)
        else:
            # both
            for d in ("c2s", "s2c"):
                try:
                    cmds = process_one(pcap, d)
                except Exception:
                    return 2
                all_cmds.append(cmds)

                print(f"\n== {pcap} ({d}) ==")
                print_summary(cmds, show_payload=args.show_payload)
                if not args.no_consecutive_diff:
                    print_consecutive_diffs(cmds, max_changes=args.max_changes)

    if args.diff_first_two and len(all_cmds) >= 2:
        print_diff_between(all_cmds[0], all_cmds[1], max_changes=args.max_changes)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
