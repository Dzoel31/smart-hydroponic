import argparse
import asyncio
import time

import pyshark


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Capture CoAP or WebSocket traffic with PyShark")
    parser.add_argument("--interface", default="Wi-Fi", help="Capture interface name, e.g. Wi-Fi")
    parser.add_argument(
        "--protocol",
        choices=["coap", "websocket"],
        required=True,
        help="Traffic type to capture",
    )
    parser.add_argument("--duration", type=int, default=60, help="Capture duration in seconds")
    parser.add_argument(
        "--output",
        default=None,
        help="Output capture file path, e.g. capture_coap.pcap",
    )
    return parser


def main() -> None:
    args = build_parser().parse_args()
    bpf_filter = "udp port 8683" if args.protocol == "coap" else "tcp port 8000"
    output_file = args.output or f"capture_{args.protocol}.pcap"

    asyncio.set_event_loop(asyncio.new_event_loop())

    capture = pyshark.LiveCapture(
        interface=args.interface,
        bpf_filter=bpf_filter,
        output_file=output_file,
    )
    start_time = time.perf_counter()
    capture.sniff(timeout=args.duration)

    captured_packets = list(capture._packets)
    capture.close()

    total_bytes = 0
    packet_count = 0
    for packet in captured_packets:
        try:
            total_bytes += int(packet.length)
            packet_count += 1
        except Exception:
            continue

    elapsed = max(time.perf_counter() - start_time, 1e-9)
    average_throughput_bps = (total_bytes * 8) / elapsed

    print(f"Total Bytes Captured: {total_bytes}")
    print(f"Average Throughput (bps): {average_throughput_bps:.2f}")
    print(f"Packets Captured: {packet_count}")
    print(f"Capture Saved To: {output_file}")


if __name__ == "__main__":
    main()