from fastapi import (
    APIRouter,
    Depends,
    Request,
    Response,
    WebSocket,
    WebSocketDisconnect,
    HTTPException,
)
from sqlalchemy.ext.asyncio import AsyncSession
from utils.deps import get_session, get_db_session, get_current_user
from services.hydroponic_service import HydroponicService
from schemas.hydroponic import (
    HydroponicIn,
    HydroponicOut,
    HydroponicDashboardOut,
    HydroponicDataSensor,
    HydroponicDataEnvironment,
    HydroponicDataActuator,
    HydroponicControlResult,
    ResponseList,
)
from schemas.user import UserOut
from uuid import uuid4
import time
from utils.manager import manager
from utils.aggregator import aggregator
from utils.evaluation_tracker import evaluation_tracker
from utils.coap_actuator_client import send_actuator_command_coap
from utils.deps import require_role
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
import logging

logging.basicConfig(
    level=logging.INFO,
    format="%(levelname)s:     %(message)s",
)
logger = logging.getLogger(__name__)

router = APIRouter(prefix="/hydroponics", tags=["Hydroponics"])

templates = Jinja2Templates(directory="./templates")

DEVICE_CONFIG = {
    "sensor-data": {
        "role": "sensor",
        "room": "hydroponics",
        "model": HydroponicDataSensor,
    },
    "environment-data": {
        "role": "environment",
        "room": "hydroponics",
        "model": HydroponicDataEnvironment,
    },
    "actuator-data": {
        "role": "actuator",
        "room": "hydroponics",
        "model": HydroponicDataActuator,
    },
    "web-client": {
        "role": "web-client",
        "room": "hydroponics",
        "model": None,
    },
}


@router.get(
    "/data/latest",
    response_model=HydroponicDashboardOut | None,
    status_code=200,
    operation_id="getLatestHydroponicData",
)
async def get_latest_hydroponic_data(
    session: AsyncSession = Depends(get_session),
) -> HydroponicDashboardOut | None:
    service = HydroponicService(session)
    data = await service.get_latest_data()

    if data is None:
        return Response(status_code=204)

    return HydroponicDashboardOut.model_validate(data)


@router.get(
    "/data/{parameter}",
    response_model=ResponseList[HydroponicOut],
    response_model_exclude_none=True,
    status_code=200,
    operation_id="getSpecificHydroponicData",
)
async def get_specific_hydroponic_data(
    parameter: str,
    page: int = 1,
    limit: int = 25,
    start_date: str | None = None,
    end_date: str | None = None,
    session: AsyncSession = Depends(get_session),
    current_user: UserOut = Depends(get_current_user),
) -> ResponseList[HydroponicOut]:
    require_role(current_user, {"admin", "superadmin"})
    service = HydroponicService(session)
    try:
        return await service.get_specific_data(
            parameter, page, limit, start_date, end_date
        )
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))


@router.post(
    "/data",
    response_model=HydroponicOut,
    status_code=201,
    operation_id="addHydroponicData",
)
async def add_hydroponic_data(
    hydroponic_data: HydroponicIn,
    session: AsyncSession = Depends(get_session),
    current_user: UserOut = Depends(get_current_user),
) -> HydroponicOut:
    """Endpoint untuk menambahkan data hidroponik baru."""
    require_role(current_user, {"admin", "superadmin"})
    service = HydroponicService(session)
    return await service.add_data(hydroponic_data)


@router.get(
    "/data",
    response_model=ResponseList[HydroponicOut],
    status_code=200,
    operation_id="getHydroponicData",
)
async def get_hydroponic_data(
    page: int = 1,
    limit: int = 25,
    start_date: str | None = None,
    end_date: str | None = None,
    current_user: UserOut = Depends(get_current_user),
    session: AsyncSession = Depends(get_session),
) -> ResponseList[HydroponicOut]:
    require_role(current_user, {"user", "admin", "superadmin"})
    service = HydroponicService(session)
    return await service.get_all_data(page, limit, start_date, end_date)


@router.post(
    "/control",
    response_model=HydroponicControlResult,
    status_code=200,
    operation_id="controlHydroponicActuators",
)
async def control_hydroponic_actuators(
    command: HydroponicDataActuator,
    transport: str = "websocket",
    current_user: UserOut = Depends(get_current_user),
) -> HydroponicControlResult:
    """Forward dashboard commands to the actuator and wait for device ACK.

    This supports Scenario 3 evaluation. The command is tagged with a
    command_id, then sent to the actuator using WebSocket or CoAP. CoAP mode is
    selected with `?transport=coap`.
    """
    require_role(current_user, {"admin", "superadmin"})
    if transport not in {"websocket", "coap"}:
        raise HTTPException(
            status_code=400,
            detail="transport must be either 'websocket' or 'coap'",
        )

    command_id = f"dashboard-{uuid4()}"
    time_start = time.time()
    command_payload = command.model_dump()

    if transport == "coap":
        coap_payload = {
            "type": "command",
            "command_id": command_id,
            "time_start": time_start,
            "payload": command_payload,
        }
        coap_result = await send_actuator_command_coap(coap_payload)
        return HydroponicControlResult(
            **command_payload,
            command_id=command_id,
            confirmed=coap_result["confirmed"],
            time_start=time_start,
            time_end=coap_result["ended_at"],
            latency_ms=coap_result["latency_ms"],
            actuator_response=coap_result,
        )

    pending = await evaluation_tracker.create(
        message_id=command_id,
        scenario="dashboard_control",
        source_role="web-client",
    )

    logger.info(f"Received control command: {command_payload}")
    await manager.send_to_room(
        room="hydroponics",
        role="actuator",
        message={
            "type": "command",
            "command_id": command_id,
            "time_start": time_start,
            "payload": command_payload,
        },
    )

    ack = await evaluation_tracker.wait_for_ack(pending, timeout=5.0)
    if ack is None:
        return HydroponicControlResult(
            **command_payload,
            command_id=command_id,
            confirmed=False,
            time_start=time_start,
        )

    return HydroponicControlResult(
        **command_payload,
        command_id=command_id,
        confirmed=True,
        time_start=time_start,
        time_end=ack["ended_at"],
        latency_ms=ack["latency_ms"],
        actuator_response=ack["actuator_payload"],
    )


@router.websocket("/ws/{device_type}")
async def hydroponic_data_websocket(device_type: str, websocket: WebSocket):
    """WebSocket endpoint untuk menerima data hidroponik secara real-time."""
    config = DEVICE_CONFIG.get(device_type)
    if not config:
        await websocket.close(code=4000, reason="Unknown device type")
        return

    session_id = str(uuid4())
    await websocket.accept()

    try:
        register = await websocket.receive_json()
        physical_id = register.get("physical_id", "unknown_device")
    except WebSocketDisconnect:
        logger.info(f"Client {session_id} disconnected before registration")
        return
    except Exception as exc:
        logger.warning(f"Invalid registration data from {session_id}: {exc}")
        try:
            await websocket.close(code=4001, reason="Invalid registration data")
        except RuntimeError:
            pass
        return

    role = config["role"]
    room = config["room"]
    validator_model = config["model"]

    await manager.connect(
        room=room, role=role, client_id=session_id, websocket=websocket
    )

    logger.info(
        f"{role.capitalize()}: {physical_id} connected with session ID: {session_id}"
    )

    try:
        while True:
            data = await websocket.receive_json()
            seq = data.get("seq", -1)
            arrival_timestamp = time.time()

            print(
                f"[SERVER_METRIC] Node: {device_type} | Seq: {seq} | Arrival_Timestamp: {arrival_timestamp}"
            )

            if role == "actuator" and data.get("type") == "actuator_ack":
                message_id = data.get("correlation_id") or data.get("command_id")
                if message_id:
                    pending = await evaluation_tracker.acknowledge(message_id, data)
                    if pending and pending.scenario == "inter_node_forward":
                        ack_result = pending.future.result() if pending.future else {}
                        await manager.send_to_client(
                            room=room,
                            role=pending.source_role,
                            client_id=pending.source_client_id,
                            message={
                                "status": "inter_node_ack",
                                "correlation_id": message_id,
                                "seq": pending.source_seq,
                                "started_at": pending.started_at,
                                "ended_at": ack_result.get("ended_at"),
                                "latency_ms": ack_result.get("latency_ms"),
                                "actuator_response": data,
                            },
                        )
                continue

            await websocket.send_json(
                {
                    "status": "ack",
                    "seq": seq,
                    "device_type": device_type,
                    "arrival_timestamp": arrival_timestamp,
                }
            )

            if validator_model:
                validated_data = validator_model.model_validate(data)

                snapshot = await aggregator.gather_data(
                    source=role,
                    data=validated_data.model_dump(),
                )

                if snapshot:
                    snapshot_payload = snapshot.model_dump()
                    snapshot_payload["seq"] = seq
                    snapshot_payload["arrival_timestamp"] = arrival_timestamp

                    async with get_db_session() as session:
                        service = HydroponicService(session)
                        saved_data = HydroponicIn.model_validate(snapshot)
                        new_data = await service.add_data(saved_data)

                    logger.info(f"Snapshot created: {new_data.model_dump()}")

                    actuator_fields = {
                        "moisture_avg",
                        "temperature_avg",
                        "pump_status",
                        "light_status",
                        "automation_status",
                    }

                    actuator_message = new_data.model_dump(include=actuator_fields)
                    actuator_message["seq"] = seq
                    actuator_message["arrival_timestamp"] = arrival_timestamp

                    correlation_id = f"sensor-{role}-{seq}-{uuid4()}"
                    await evaluation_tracker.create(
                        message_id=correlation_id,
                        scenario="inter_node_forward",
                        source_role=role,
                        source_client_id=session_id,
                        source_seq=seq,
                    )
                    actuator_message = {
                        "type": "sensor_forward",
                        "correlation_id": correlation_id,
                        "source_role": role,
                        "source_seq": seq,
                        "forward_timestamp": time.time(),
                        "payload": actuator_message,
                    }

                    await manager.send_to_room(
                        room=room, role="web-client", message=snapshot_payload
                    )

                    await manager.send_to_room(
                        room=room,
                        role="actuator",
                        message=actuator_message,
                    )
                    logger.info(
                        f"Snapshot created and sent to actuator clients: {actuator_message}"
                    )
            else:
                # Directly forward commands from dashboard to actuators
                await manager.send_to_room(
                    room=room,
                    role="actuator",
                    message={"type": "command", "payload": data},
                )

    except WebSocketDisconnect:
        await manager.disconnect(room=room, role=role, client_id=session_id)
        logger.info(f"Client {session_id} disconnected")

    except Exception as e:
        await manager.disconnect(room=room, role=role, client_id=session_id)
        try:
            await websocket.close(code=1011)
        except RuntimeError:
            pass
        logger.error(f"Error: {e}")


@router.get("/test-sensor-data", response_class=HTMLResponse)
async def test_sensor_data(request: Request):
    """Endpoint untuk menguji WebSocket sensor data hidroponik."""
    return templates.TemplateResponse("test_ws_sensor_data.html", {"request": request})
