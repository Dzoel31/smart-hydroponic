import aiocoap.resource as resource
import aiocoap
import json
from schemas import (
    HydroponicDataSensor,
    HydroponicDataEnvironment,
    HydroponicDataActuator,
)
from utils.deps import get_db_session
from utils.aggregator import HydroponicAggregator
from utils.manager import RoomConnectionManager
from services.hydroponic_service import HydroponicService

COAP_CONFIG = {
    "sensor": HydroponicDataSensor,
    "environment": HydroponicDataEnvironment,
    "actuator": HydroponicDataActuator,
    "web-client": None,
}


class HydroponicCoAPResource(resource.Resource):
    def __init__(
        self,
        role: str,
        aggregator: HydroponicAggregator,
        manager: RoomConnectionManager,
    ):
        super().__init__()
        self.role = role
        self.aggregator = aggregator
        self.manager = manager
        self.validator = COAP_CONFIG.get(role)

    async def render_put(self, request):
        try:
            payload = request.payload.decode("utf-8")
            data_json = json.loads(payload)

            print(f"[COAP] Received data for role '{self.role}': {data_json}")

            if self.validator:
                data = self.validator.model_validate(data_json)
                snapshot = await self.aggregator.gather_data(
                    self.role, data.model_dump()
                )

                if snapshot:
                    async with get_db_session() as session:
                        service = HydroponicService(session)
                        await service.add_data(snapshot)
                    await self.manager.send_to_room(
                        room="hydroponics",
                        role="web-client",
                        message=snapshot.model_dump(),
                    )
                    print("[COAP] Snapshot complete and Broadcasted!")

            return aiocoap.Message(code=aiocoap.CHANGED, payload=b"Data received")

        except Exception as e:
            print(f"[COAP][ERROR] {e}")
            return aiocoap.Message(
                code=aiocoap.INTERNAL_SERVER_ERROR, payload=b"Error processing data"
            )
