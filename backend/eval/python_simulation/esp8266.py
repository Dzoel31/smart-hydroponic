import argparse
import asyncio
import csv
import json
import random
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

import websockets
from aiocoap import Context, Message, PUT

try:
    from tqdm import tqdm
except ImportError:
    tqdm = None

PROJECT_ROOT = Path(__file__).resolve().parents[1]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

try:
    from ..utils.models import ActuatorData
except ImportError:
    from utils.models import ActuatorData

CONFIG = {
    "coap_url": "coap://localhost:5683/coap/hydroponics/actuator",
    "websocket_url": "http://127.0.0.1:8000/smart-hydroponic/api/v2/hydroponics/ws/actuators",
    "interval": ["1s", "5s", "10s", "15s", "30s", "1m"],
    "duration": ["1m", "5m", "10m", "15m", "30m", "1h"],
}


def now_utc_iso():
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds")


def parse_duration_to_seconds(duration_text):
    value = int(duration_text[:-1])
    unit = duration_text[-1]

    if unit == "s":
        return value
    if unit == "m":
        return value * 60
    if unit == "h":
        return value * 3600

    raise ValueError(f"Unsupported time format: {duration_text}")


def select_option(title, options):
    print(f"\n{title}")
    for idx, option in enumerate(options, 1):
        print(f"{idx}. {option}")

    selected = input("Select: ").strip()
    if not selected.isdigit():
        print("Invalid input. Enter a number.")
        return None

    index = int(selected) - 1
    if index < 0 or index >= len(options):
        print("Invalid selection.")
        return None

    return options[index]


def create_progress_bar(duration_seconds, enabled):
    if not enabled:
        return None
    if tqdm is None:
        print("tqdm not installed; continuing without progress bar.")
        return None
    return tqdm(total=duration_seconds, unit="s", desc="esp8266", leave=False)


def create_log_file(protocol, node, coap_path, interval_text, duration_text):
    logs_dir = PROJECT_ROOT / "data" / "simulation_logs"
    logs_dir.mkdir(parents=True, exist_ok=True)
    endpoint = coap_path if protocol == "coap" else "ws"
    file_name = (
        f"esp8266_{protocol}_{endpoint}_{node}_{interval_text}_{duration_text}_{int(time.time())}.csv"
    )
    file_path = logs_dir / file_name
    file_handle = open(file_path, "w", newline="", encoding="utf-8")
    writer = csv.DictWriter(
        file_handle,
        fieldnames=[
            "run_id",
            "msg_id",
            "device",
            "protocol",
            "node",
            "endpoint",
            "time_send",
            "time_response",
            "latency_ms",
            "jitter_ms",
            "status",
            "error",
            "interval",
            "duration",
        ],
    )
    writer.writeheader()
    return file_handle, writer, file_path


async def send_coap_request(payload, uri, verbose):
    started_at = time.perf_counter()
    context = await Context.create_client_context()
    request = Message(code=PUT, payload=json.dumps(payload).encode("utf-8"), uri=uri)

    try:
        response = await context.request(request).response
        response_time = now_utc_iso()
        latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
        if verbose:
            print(f"CoAP response: {response.payload.decode('utf-8', errors='ignore')}")
        return {
            "time_response": response_time,
            "latency_ms": latency_ms,
            "status": "ok",
            "error": "",
        }
    except Exception as exc:
        response_time = now_utc_iso()
        latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
        print(f"CoAP send error: {exc}")
        return {
            "time_response": response_time,
            "latency_ms": latency_ms,
            "status": "error",
            "error": str(exc),
        }
    finally:
        await context.shutdown()


async def send_websocket_message(websocket, node, payload, verbose):
    started_at = time.perf_counter()
    try:
        await websocket.send(json.dumps(payload))
        try:
            response = await asyncio.wait_for(websocket.recv(), timeout=5)
            response_time = now_utc_iso()
            latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
            if verbose:
                print(f"WebSocket response ({node}): {response}")
            return {
                "time_response": response_time,
                "latency_ms": latency_ms,
                "status": "ok",
                "error": "",
            }
        except asyncio.TimeoutError:
            response_time = now_utc_iso()
            latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
            print(f"No WebSocket response from {node} within 5s.")
            return {
                "time_response": response_time,
                "latency_ms": latency_ms,
                "status": "timeout",
                "error": "timeout waiting websocket response",
            }
        except (websockets.exceptions.ConnectionClosed, OSError) as exc:
            response_time = now_utc_iso()
            latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
            print(f"WebSocket connection lost for {node}: {exc}")
            return {
                "time_response": response_time,
                "latency_ms": latency_ms,
                "status": "connection_lost",
                "error": str(exc),
            }
    except (websockets.exceptions.ConnectionClosed, OSError) as exc:
        response_time = now_utc_iso()
        latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
        print(f"WebSocket connection lost for {node}: {exc}")
        return {
            "time_response": response_time,
            "latency_ms": latency_ms,
            "status": "connection_lost",
            "error": str(exc),
        }
    except Exception as exc:
        response_time = now_utc_iso()
        latency_ms = round((time.perf_counter() - started_at) * 1000, 3)
        print(f"WebSocket send error: {exc}")
        return {
            "time_response": response_time,
            "latency_ms": latency_ms,
            "status": "error",
            "error": str(exc),
        }


def generate_dummy_data(message_id):
    return ActuatorData(
        message_id=message_id,
        pump_status=random.choice([True, False]),
        light_status=random.choice([True, False]),
        automation_status=random.choice([True, False]),
    ).model_dump()


async def run_simulation(
    protocol,
    node,
    coap_path,
    interval_text,
    duration_text,
    interval_seconds,
    duration_seconds,
    verbose=False,
    show_progress=False,
):
    run_id = f"esp8266-{int(time.time() * 1000)}"
    log_file, log_writer, log_path = create_log_file(
        protocol, node, coap_path, interval_text, duration_text
    )
    progress_bar = create_progress_bar(duration_seconds, show_progress)
    last_progress = 0.0
    start_time = time.monotonic()
    sent_count = 0
    prev_latency_ms = None
    websocket = None
    reconnect_attempted = False
    websocket_uri = f"{CONFIG['websocket_url']}/{node}" if node else CONFIG["websocket_url"]

    try:
        if protocol == "websocket":
            websocket = await websockets.connect(websocket_uri)

        while (time.monotonic() - start_time) < duration_seconds:
            sent_count += 1
            msg_id = sent_count
            time_send = now_utc_iso()
            payload = generate_dummy_data(msg_id)

            if protocol == "coap":
                telemetry = await send_coap_request(payload, CONFIG["coap_url"][coap_path], verbose)
            else:
                telemetry = await send_websocket_message(websocket, node, payload, verbose)
                if telemetry["status"] == "connection_lost" and not reconnect_attempted:
                    reconnect_attempted = True
                    print("WebSocket disconnected. Attempting one-time reconnect...")
                    try:
                        if websocket is not None:
                            await websocket.close()
                    except Exception:
                        pass

                    try:
                        websocket = await websockets.connect(websocket_uri)
                        telemetry = await send_websocket_message(websocket, node, payload, verbose)
                    except Exception as reconnect_exc:
                        print(f"WebSocket reconnect failed: {reconnect_exc}")
                        telemetry = {
                            "time_response": now_utc_iso(),
                            "latency_ms": 0.0,
                            "status": "error",
                            "error": f"reconnect failed: {reconnect_exc}",
                        }

            latency_ms = telemetry["latency_ms"]
            jitter_ms = None
            if prev_latency_ms is not None:
                jitter_ms = round(abs(latency_ms - prev_latency_ms), 3)
            prev_latency_ms = latency_ms

            log_writer.writerow(
                {
                    "run_id": run_id,
                    "msg_id": msg_id,
                    "device": "esp8266",
                    "protocol": protocol,
                    "node": node,
                    "endpoint": coap_path if protocol == "coap" else "ws",
                    "time_send": time_send,
                    "time_response": telemetry["time_response"],
                    "latency_ms": latency_ms,
                    "jitter_ms": jitter_ms if jitter_ms is not None else "",
                    "status": telemetry["status"],
                    "error": telemetry["error"],
                    "interval": interval_text,
                    "duration": duration_text,
                }
            )
            log_file.flush()

            elapsed = time.monotonic() - start_time
            if progress_bar is not None:
                capped = min(duration_seconds, elapsed)
                increment = capped - last_progress
                if increment > 0:
                    progress_bar.update(increment)
                    last_progress = capped

            remaining = duration_seconds - elapsed
            if remaining <= 0:
                break

            await asyncio.sleep(min(interval_seconds, remaining))
    finally:
        if websocket is not None:
            await websocket.close()
        if progress_bar is not None:
            if last_progress < duration_seconds:
                progress_bar.update(duration_seconds - last_progress)
            progress_bar.close()
        log_file.close()

    print(f"Simulation complete. Packets sent: {sent_count}")
    print(f"Telemetry CSV saved: {log_path}")


def run_once(protocol, node, coap_path, interval_text, duration_text, verbose, show_progress):
    interval_seconds = parse_duration_to_seconds(interval_text)
    duration_seconds = parse_duration_to_seconds(duration_text)
    asyncio.run(
        run_simulation(
            protocol=protocol,
            node=node,
            coap_path=coap_path,
            interval_text=interval_text,
            duration_text=duration_text,
            interval_seconds=interval_seconds,
            duration_seconds=duration_seconds,
            verbose=verbose,
            show_progress=show_progress,
        )
    )


def run_interactive():
    while True:
        protocol = select_option("Select communication protocol:", ["coap", "websocket"])
        if protocol is None:
            continue

        interval_text = select_option("Select interval:", CONFIG["interval"])
        if interval_text is None:
            continue

        duration_text = select_option("Select duration:", CONFIG["duration"])
        if duration_text is None:
            continue

        node = "actuators"
        coap_path = "control"

        if protocol == "coap":
            selected_path = select_option("Select CoAP endpoint:", ["control", "status"])
            if selected_path is None:
                continue
            coap_path = selected_path
            node = input("Enter logical node name for logs (default: actuators): ").strip() or "actuators"
        else:
            node = input("Enter WebSocket node name (default: actuators): ").strip() or "actuators"

        print(
            f"\nRunning {protocol} simulation for {duration_text} with {interval_text} interval..."
        )
        try:
            run_once(protocol, node, coap_path, interval_text, duration_text, verbose=False, show_progress=False)
        except KeyboardInterrupt:
            print("\nSimulation interrupted by user.")

        rerun = input("\nRun another simulation? (y/n): ").strip().lower()
        if rerun != "y":
            break


def build_arg_parser():
    parser = argparse.ArgumentParser(description="ESP8266 actuator simulator")
    parser.add_argument("--protocol", choices=["coap", "websocket"], help="Communication protocol")
    parser.add_argument("--node", default="actuators", help="Node suffix for websocket path")
    parser.add_argument("--coap-path", choices=["control", "status"], default="control", help="CoAP endpoint")
    parser.add_argument("--interval", default="1s", help="Send interval, e.g. 1s, 5s, 1m")
    parser.add_argument("--duration", default="1m", help="Run duration, e.g. 1m, 30s")
    parser.add_argument("--verbose", action="store_true", help="Print per-message responses")
    parser.add_argument("--progress", action="store_true", help="Show tqdm progress bar")
    parser.add_argument("--interactive", action="store_true", help="Run interactive mode")
    return parser


def main():
    args = build_arg_parser().parse_args()

    if args.interactive or args.protocol is None:
        run_interactive()
        return

    try:
        run_once(
            protocol=args.protocol,
            node=args.node,
            coap_path=args.coap_path,
            interval_text=args.interval,
            duration_text=args.duration,
            verbose=args.verbose,
            show_progress=args.progress,
        )
    except KeyboardInterrupt:
        print("\nSimulation interrupted by user.")


if __name__ == "__main__":
    main()