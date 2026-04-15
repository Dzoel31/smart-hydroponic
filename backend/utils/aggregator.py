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
    def __init__(self, timeout: float = 5.0, min_interval: float = 1.0):
        self.buffer = {"sensor": None, "environment": None, "actuator": None}
        self.last_update = time.monotonic()
        self.timeout = timeout
        self.last_received = {}
        self.min_interval = min_interval
        self.lock = asyncio.Lock()

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
                logger.warning("Incomplete data; resetting buffer due to timeout.")
                self.reset()

            self.buffer[source] = data
            self.last_update = now

            if not self.is_complete():
                return None

            snapshot = self.build_snapshot()
            self.reset()
            return snapshot

    def is_complete(self):
        return all(value is not None for value in self.buffer.values())

    def build_snapshot(self) -> HydroponicIn:
        combined_data = {
            **self.buffer["sensor"],
            **self.buffer["environment"],
            **self.buffer["actuator"],
        }

        return HydroponicIn(dataid=uuid7(), **combined_data)

    def reset(self):
        # Reset buffer
        self.buffer = {"sensor": None, "environment": None, "actuator": None}
        self.last_update = time.monotonic()


aggregator = HydroponicAggregator()
