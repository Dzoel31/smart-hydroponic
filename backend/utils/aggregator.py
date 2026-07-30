import asyncio
from schemas import HydroponicIn
from uuid import uuid7
import time
import logging

logging.basicConfig(
    level=logging.INFO,
    format="%(levelname)s:     %(message)s",
)
logger = logging.getLogger(__name__)

logging.basicConfig(
    level=logging.INFO,
    format="%(levelname)s:     %(message)s",
)
logger = logging.getLogger(__name__)


class HydroponicAggregator:
    def __init__(self, timeout: float = 60.0, min_interval: float = 1.0):
        self.buffer = {"plant": None, "environment": None, "actuator": None}
        self.last_update = time.monotonic()
        self.timeout = timeout
        self.last_received = {}
        self.min_interval = min_interval
        self.lock = asyncio.Lock()

    def debug_buffer(self, source: str, event: str):
        if event == "after-update":
            logger.info(f"[Aggregator] Menerima data dari node: {source}")
        elif event == "snapshot-ready":
            logger.info(
                "[Aggregator] Semua data dari 3 node terkumpul! Membuat snapshot ke database."
            )

    async def gather_data(self, source: str, data: dict) -> HydroponicIn | None:
        async with self.lock:
            now = time.monotonic()

            if source in self.last_received:
                delta = now - self.last_received[source]
                if delta < self.min_interval:
                    logger.warning(
                        f"Data from {source} received too quickly ({delta:.2f}s); ignoring."
                    )
                    return None

            self.last_received[source] = now

            if any(self.buffer.values()) and (now - self.last_update > self.timeout):
                logger.warning(
                    "Incomplete data; resetting buffer due to timeout "
                    f"({now - self.last_update:.2f}s > {self.timeout:.2f}s)."
                )
                self.reset()

            self.buffer[source] = data
            self.last_update = now
            self.debug_buffer(source, "after-update")

            if not self.is_complete():
                return None

            self.debug_buffer(source, "snapshot-ready")
            snapshot = self.build_snapshot()
            self.reset()
            return snapshot

    def is_complete(self):
        return all(value is not None for value in self.buffer.values())

    def build_snapshot(self) -> HydroponicIn:
        combined_data = {
            **self.buffer["plant"],
            **self.buffer["environment"],
            **self.buffer["actuator"],
        }

        return HydroponicIn(dataid=uuid7(), **combined_data)

    def reset(self):
        # Reset buffer
        self.buffer = {"plant": None, "environment": None, "actuator": None}
        self.last_update = time.monotonic()


aggregator = HydroponicAggregator(timeout=60.0)
