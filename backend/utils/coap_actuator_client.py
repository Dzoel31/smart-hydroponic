import json
import os
import time
from typing import Any

import aiocoap


def get_actuator_control_uri() -> str:
    """Build the ESP8266 actuator CoAP endpoint URI.

    The defaults match the evaluation firmware in
    `backend/eval/esp/coap/esp8266/esp8266.ino`. Override them with environment
    variables when the actuator uses a different IP, port, or path.
    """

    host = os.getenv("COAP_ACTUATOR_HOST", "172.25.21.236")
    port = os.getenv("COAP_ACTUATOR_PORT", "5683")
    path = os.getenv("COAP_ACTUATOR_CONTROL_PATH", "actuator/control")
    return f"coap://{host}:{port}/{path}"


async def send_actuator_command_coap(
    payload: dict[str, Any],
    uri: str | None = None,
) -> dict[str, Any]:
    """Send a command/forwarded snapshot to ESP8266 Actuator via CoAP.

    Returns timing metadata measured at the backend application level. The
    actuator response payload is decoded as JSON when possible so it can be
    stored together with the QoS result.
    """

    target_uri = uri or get_actuator_control_uri()
    started_at = time.time()
    context = await aiocoap.Context.create_client_context()
    request = aiocoap.Message(
        code=aiocoap.PUT,
        payload=json.dumps(payload).encode("utf-8"),
        uri=target_uri,
    )

    try:
        response = await context.request(request).response
        ended_at = time.time()
        raw_payload = response.payload.decode("utf-8", errors="ignore")
        try:
            response_payload: dict[str, Any] = json.loads(raw_payload)
        except json.JSONDecodeError:
            response_payload = {"raw_payload": raw_payload}

        return {
            "confirmed": True,
            "uri": target_uri,
            "started_at": started_at,
            "ended_at": ended_at,
            "latency_ms": round((ended_at - started_at) * 1000, 3),
            "response_code": str(response.code),
            "actuator_payload": response_payload,
        }
    except Exception as exc:
        ended_at = time.time()
        return {
            "confirmed": False,
            "uri": target_uri,
            "started_at": started_at,
            "ended_at": ended_at,
            "latency_ms": round((ended_at - started_at) * 1000, 3),
            "error": str(exc),
            "actuator_payload": None,
        }
    finally:
        await context.shutdown()
