import asyncio
import time
from dataclasses import dataclass, field
from typing import Any


@dataclass
class PendingEvaluationMessage:
    message_id: str
    scenario: str
    source_role: str
    source_client_id: str | None
    source_seq: int | None
    started_at: float = field(default_factory=time.time)
    future: asyncio.Future[dict[str, Any]] | None = None


class EvaluationTracker:
    """Tracks forwarded messages until the actuator confirms execution.

    The tracker is intentionally kept in memory because it is used for short-lived
    QoS experiments, not business state. Every pending item is keyed by a
    correlation id that is sent to the actuator and echoed back in its ACK.
    """

    def __init__(self) -> None:
        self._pending: dict[str, PendingEvaluationMessage] = {}
        self._lock = asyncio.Lock()

    async def create(
        self,
        message_id: str,
        scenario: str,
        source_role: str,
        source_client_id: str | None = None,
        source_seq: int | None = None,
    ) -> PendingEvaluationMessage:
        loop = asyncio.get_running_loop()
        pending = PendingEvaluationMessage(
            message_id=message_id,
            scenario=scenario,
            source_role=source_role,
            source_client_id=source_client_id,
            source_seq=source_seq,
            future=loop.create_future(),
        )

        async with self._lock:
            self._pending[message_id] = pending

        return pending

    async def acknowledge(
        self,
        message_id: str,
        actuator_payload: dict[str, Any],
    ) -> PendingEvaluationMessage | None:
        async with self._lock:
            pending = self._pending.pop(message_id, None)

        if pending is None:
            return None

        ended_at = time.time()
        result = {
            "message_id": message_id,
            "scenario": pending.scenario,
            "source_role": pending.source_role,
            "source_client_id": pending.source_client_id,
            "source_seq": pending.source_seq,
            "started_at": pending.started_at,
            "ended_at": ended_at,
            "latency_ms": round((ended_at - pending.started_at) * 1000, 3),
            "actuator_payload": actuator_payload,
        }

        if pending.future is not None and not pending.future.done():
            pending.future.set_result(result)

        return pending

    async def wait_for_ack(
        self,
        pending: PendingEvaluationMessage,
        timeout: float,
    ) -> dict[str, Any] | None:
        if pending.future is None:
            return None

        try:
            return await asyncio.wait_for(pending.future, timeout=timeout)
        except asyncio.TimeoutError:
            async with self._lock:
                self._pending.pop(pending.message_id, None)
            return None


evaluation_tracker = EvaluationTracker()
