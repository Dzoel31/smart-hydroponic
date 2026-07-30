import argparse
import csv
import os
import re
import threading
import time
from datetime import datetime
from queue import Queue, Empty

import serial


METRIC_PATTERNS = [
    re.compile(
        r"\[(?P<metric_type>TX)\]\s+Seq:\s*(?P<seq>\d+)"
    ),
    re.compile(
        r"\[(?P<metric_type>S1_METRIC|METRIC)\]\s+Seq:\s*(?P<seq>\d+)\s+\|\s+Latency:\s*(?P<latency>[0-9]+(?:\.[0-9]+)?)\s*ms"
    ),
    re.compile(
        r"\[(?P<metric_type>S2_METRIC)\]\s+Seq:\s*(?P<seq>\d+)\s+\|\s+EndToEndLatency:\s*(?P<latency>[0-9]+(?:\.[0-9]+)?)\s*ms"
    ),
    re.compile(
        r"\[(?P<metric_type>S2_AGG_METRIC)\]\s+SourceSeq:\s*(?P<seq>\d+)\s+\|\s+AckTimeMs:\s*(?P<ack_time_ms>\d+)\s+\|\s+Correlation:\s*(?P<command_id>.+)"
    ),
    re.compile(
        r"\[(?P<metric_type>S3_METRIC)\]\s+CommandId:\s*(?P<command_id>[^|]+)\|\s+AckTimeMs:\s*(?P<ack_time_ms>\d+)"
    ),
]


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Read ESP32 metrics from Serial and save to CSV")
    parser.add_argument("--port", default=None, help="Single serial port, e.g. COM3")
    parser.add_argument(
        "--ports",
        nargs="+",
        default=None,
        help="Multiple serial ports, e.g. COM3 COM4 COM5",
    )
    parser.add_argument(
        "--labels",
        nargs="+",
        default=None,
        help="Optional labels for each port, same order as --ports",
    )
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate")
    parser.add_argument("--output", default="esp32_latency_log.csv", help="CSV output file")
    parser.add_argument(
        "--duration",
        type=int,
        default=None,
        help="Optional logging duration in seconds, e.g. 3600 for one hour",
    )
    return parser


def ensure_header(path: str) -> None:
    if os.path.exists(path) and os.path.getsize(path) > 0:
        return

    with open(path, "w", newline="", encoding="utf-8") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(
            [
                "logged_at",
                "source_port",
                "device_label",
                "metric_type",
                "seq",
                "command_id",
                "latency_ms",
                "ack_time_ms",
                "raw_line",
            ]
        )


def parse_metric_line(raw_line: str):
    for pattern in METRIC_PATTERNS:
        match = pattern.search(raw_line)
        if not match:
            continue

        groups = match.groupdict()
        return {
            "metric_type": groups.get("metric_type"),
            "seq": groups.get("seq", ""),
            "command_id": (groups.get("command_id") or "").strip(),
            "latency_ms": groups.get("latency", ""),
            "ack_time_ms": groups.get("ack_time_ms", ""),
        }

    return None


def serial_reader(port: str, baud: int, device_label: str, queue: Queue, stop_event: threading.Event) -> None:
    try:
        with serial.Serial(port, baud, timeout=1) as ser:
            while not stop_event.is_set():
                raw_line = ser.readline().decode(errors="ignore").strip()
                if raw_line:
                    queue.put((port, device_label, raw_line))
    except KeyboardInterrupt:
        return
    except Exception as exc:
        queue.put((port, device_label, f"[ERROR] {exc}"))


def main() -> None:
    args = build_parser().parse_args()
    if args.duration is not None and args.duration <= 0:
        raise SystemExit("--duration must be greater than zero")

    ensure_header(args.output)

    ports = args.ports or ([args.port] if args.port else [])
    if not ports:
        raise SystemExit("Specify --port or --ports")

    labels = args.labels or []
    if labels and len(labels) != len(ports):
        raise SystemExit("When using --labels, provide the same number of labels as ports")

    device_labels = [
        labels[i] if i < len(labels) else f"device_{i + 1}"
        for i in range(len(ports))
    ]

    for port, label in zip(ports, device_labels):
        print(f"Opening {port} as {label} at {args.baud} baud")

    queue: Queue = Queue()
    stop_event = threading.Event()
    threads: list[threading.Thread] = []
    for port, label in zip(ports, device_labels):
        thread = threading.Thread(
            target=serial_reader,
            args=(port, args.baud, label, queue, stop_event),
            daemon=True,
        )
        thread.start()
        threads.append(thread)

    started_at = time.monotonic()
    if args.duration is not None:
        print(f"Logging for {args.duration} seconds")

    try:
        with open(args.output, "a", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)

            while True:
                if args.duration is not None and time.monotonic() - started_at >= args.duration:
                    print(f"\nReached logging duration: {args.duration} seconds.")
                    break

                try:
                    source_port, device_label, raw_line = queue.get(timeout=1)
                except Empty:
                    continue

                print(f"[{device_label}] {raw_line}")

                if "_METRIC]" not in raw_line and "[METRIC]" not in raw_line and "[TX]" not in raw_line:
                    continue

                parsed = parse_metric_line(raw_line)
                if not parsed:
                    continue

                logged_at = datetime.now().isoformat(timespec="seconds")
                writer.writerow(
                    [
                        logged_at,
                        source_port,
                        device_label,
                        parsed["metric_type"],
                        parsed["seq"],
                        parsed["command_id"],
                        parsed["latency_ms"],
                        parsed["ack_time_ms"],
                        raw_line,
                    ]
                )
                csv_file.flush()

    except KeyboardInterrupt:
        print("\nStopping logger and closing serial ports.")
    finally:
        stop_event.set()
        for thread in threads:
            thread.join(timeout=2)


if __name__ == "__main__":
    main()
