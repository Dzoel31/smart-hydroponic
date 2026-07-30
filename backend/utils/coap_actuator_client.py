import asyncio
import json
import os
from typing import Any

import aiocoap
import logging

logger = logging.getLogger(__name__)


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


def get_actuator_timeout_seconds() -> float:
    return float(os.getenv("COAP_ACTUATOR_TIMEOUT_SECONDS", "5"))


async def send_actuator_command_coap(
    payload: dict[str, Any],
    uri: str | None = None,
) -> dict[str, Any]:
    """Send a command/forwarded snapshot to ESP8266 Actuator via CoAP."""

    target_uri = uri or get_actuator_control_uri()
    timeout_seconds = get_actuator_timeout_seconds()

    context = await aiocoap.Context.create_client_context()
    request = aiocoap.Message(
        code=aiocoap.PUT,
        payload=json.dumps(payload).encode("utf-8"),
        uri=target_uri,
    )

    try:
        logger.info(
            "[COAP_FORWARD] Sending actuator command | "
            f"URI: {target_uri} | Timeout: {timeout_seconds}s"
        )
        response = await asyncio.wait_for(
            context.request(request).response,
            timeout=timeout_seconds,
        )

        raw_payload = response.payload.decode("utf-8", errors="ignore")
        try:
            response_payload: dict[str, Any] = json.loads(raw_payload)
        except json.JSONDecodeError:
            response_payload = {"raw_payload": raw_payload}

        return {
            "confirmed": True,
            "response_code": str(response.code),
            "actuator_payload": response_payload,
        }
    except TimeoutError:
        return {
            "confirmed": False,
            "error": f"CoAP actuator request timed out after {timeout_seconds}s",
        }
    except Exception as exc:
        return {
            "confirmed": False,
            "error": str(exc),
        }
    finally:
        await context.shutdown()
