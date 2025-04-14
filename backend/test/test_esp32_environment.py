import websockets
import random
import dotenv
import os
import asyncio
import json

dotenv.load_dotenv()

DEVICE_ID = "esp32-environment-device"
uri = f"ws://{os.getenv('HOST')}:{os.getenv('PORT')}/ws/smart-hydroponic/device"

def data_environment():
    return {
        "device_id": "esp32-environment-device",
        "type": "update_data",
        "data": {
            "temperatureAtas": 20,
            "temperatureBawah": 20,
            "humidityAtas": random.randint(1, 100),
            "humidityBawah": random.randint(1, 100),
        },
    }

async def send_pong(websocket):
    while True:
        try:
            await asyncio.sleep(5)  # Wait for 5 seconds before sending pong
            await websocket.send("pong")
            print("Sent pong")
        except websockets.exceptions.ConnectionClosedError:
            print("Connection closed, stopping pong sender.")
            break

async def main():
    while True:
        try:
            ws = websockets.connect(uri)
            async with ws as websocket:
                register_data = {
                    "device_id": DEVICE_ID,
                    "type": "register",
                }
                await websocket.send(json.dumps(register_data))
                print(f"Sent register data: {register_data}")
                while True:
                    data = data_environment()
                    json_data = json.dumps(data)
                    await websocket.send(json_data)
                    print(f"Sent data: {json_data}\n")
                    await asyncio.sleep(
                        5
                    )  # Delay of 5 seconds before sending the next data
        except (
            websockets.exceptions.ConnectionClosedError,
            ConnectionRefusedError,
        ) as e:
            print(f"Connection error: {e}. Reconnecting in 5 seconds...")
            await asyncio.sleep(5)


if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())