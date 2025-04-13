import websockets
import random
import dotenv
import os
import asyncio
import json
from websockets.exceptions import ConnectionClosedError

dotenv.load_dotenv()

DEVICE_ID = "esp8266-actuator-device"
uri = f"ws://{os.getenv('IP4_ADDRESS')}:{os.getenv('PORT')}/device"

pump_status = 0
lamp_status = 0
automation_status = 0

async def send_pong(websocket):
    """
    Sends a 'pong' message to the server every 5 seconds.
    """
    while True:
        try:
            await asyncio.sleep(5)  # Wait for 5 seconds before sending pong
            await websocket.pong()
            print("Sent pong")
        except websockets.exceptions.ConnectionClosedError:
            print("Connection closed, stopping pong sender.")
            break

def handle_actuator_response(data):
    """
    Handles the actuator response from the server.
    Updates the actuator status based on the received data.
    """
    global pump_status, lamp_status, automation_status

    print("Received data:", data)

async def send_device_data(websocket):
    """
    Sends the current status of the device to the server.
    """
    global pump_status, lamp_status, automation_status

    data = {
        "device_id": DEVICE_ID,
        "type": "update_data",
        "data": {
            "pumpStatus": pump_status,
            "lightStatus": lamp_status,
            "automationStatus": automation_status,
        },
    }
    json_data = json.dumps(data)
    await websocket.send(json_data)
    print(f"Sent data: {json_data}\n")

async def receive_device_data(websocket):
    """
    Receives data from the server and returns it as a JSON object.
    """
    try:
        data = await websocket.recv()
        return json.loads(data)
    except websockets.exceptions.ConnectionClosedError:
        print("Connection closed, stopping receiver.")
        return None

async def websocket_handler() -> None:
    """
    Handles the websocket connection and communication.
    Continuously sends the current status and processes received data.
    """
    while True:
        try:
            async with websockets.connect(uri) as websocket:
                # Start a task to send periodic 'pong' messages
                register_data = {
                    "device_id": DEVICE_ID,
                    "type": "register",
                }
                await websocket.send(json.dumps(register_data))
                print(f"Sent register data: {register_data}")
                pong_task = asyncio.create_task(send_pong(websocket))

                while True:
                    # Send the current device status
                    await send_device_data(websocket)

                    # Receive and handle data from the server
                    data = await receive_device_data(websocket)
                    handle_actuator_response(data)

                    # Delay before the next iteration
                    await asyncio.sleep(5)

        except (ConnectionClosedError, ConnectionRefusedError) as e:
            print(f"Connection error: {e}. Reconnecting in 5 seconds...")
            await asyncio.sleep(5)


if __name__ == "__main__":
    asyncio.run(websocket_handler())
