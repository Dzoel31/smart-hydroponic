import argparse
import csv
import os
import re
import threading
from datetime import datetime
from queue import Queue, Empty

import serial


METRIC_PATTERN = re.compile(r"\[METRIC\]\s+Seq:\s*(\d+)\s+\|\s+Latency:\s*([0-9]+(?:\.[0-9]+)?)\s*ms")


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
    return parser


def ensure_header(path: str) -> None:
    if os.path.exists(path) and os.path.getsize(path) > 0:
        return

    with open(path, "w", newline="", encoding="utf-8") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["logged_at", "source_port", "device_label", "seq", "latency_ms", "raw_line"])


def parse_metric_line(raw_line: str):
    match = METRIC_PATTERN.search(raw_line)
    if not match:
        return None

    return int(match.group(1)), float(match.group(2))


def serial_reader(port: str, baud: int, device_label: str, queue: Queue) -> None:
    try:
        with serial.Serial(port, baud, timeout=1) as ser:
            while True:
                raw_line = ser.readline().decode(errors="ignore").strip()
                if raw_line:
                    queue.put((port, device_label, raw_line))
    except KeyboardInterrupt:
        return
    except Exception as exc:
        queue.put((port, device_label, f"[ERROR] {exc}"))


def main() -> None:
    args = build_parser().parse_args()
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
    threads: list[threading.Thread] = []
    for port, label in zip(ports, device_labels):
        thread = threading.Thread(target=serial_reader, args=(port, args.baud, label, queue), daemon=True)
        thread.start()
        threads.append(thread)

    try:
        with open(args.output, "a", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)

            while True:
                try:
                    source_port, device_label, raw_line = queue.get(timeout=1)
                except Empty:
                    continue

                print(f"[{device_label}] {raw_line}")

                if "[METRIC]" not in raw_line:
                    continue

                parsed = parse_metric_line(raw_line)
                if not parsed:
                    continue

                seq, latency_ms = parsed
                logged_at = datetime.now().isoformat(timespec="seconds")
                writer.writerow([logged_at, source_port, device_label, seq, latency_ms, raw_line])
                csv_file.flush()

    except KeyboardInterrupt:
        print("\nStopping logger and closing serial ports.")


if __name__ == "__main__":
    main()