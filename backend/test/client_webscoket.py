import websockets
import random
import dotenv
import os
import asyncio
import json

dotenv.load_dotenv()


def data_plant():
    return {
        "device_id": "esp32-plant-device",
        "data": {
            "moisture1": random.randint(0, 100),
            "moisture2": random.randint(0, 100),
            "moisture3": random.randint(0, 100),
            "moisture4": random.randint(0, 100),
            "moisture5": random.randint(0, 100),
            "moisture6": random.randint(0, 100),
            "flowRate": random.randint(0, 100),
            "totalLitres": random.randint(0, 100),
            "distanceCm": random.randint(0, 100),
        },
    }


def data_environment():
    return {
        "device_id": "esp32-environment-device",
        "data": {
            "temperatureAtas": random.randint(0, 100),
            "temperatureBawah": random.randint(0, 100),
            "humidityAtas": random.randint(0, 100),
            "humidityBawah": random.randint(0, 100),
        },
    }


def data_actuator():
    return {
        "device_id": "esp8266-actuator-device",
        "data": {
            "pumpStatus": random.choice([0, 1]),
            "lightStatus": random.choice([0, 1]),
            "automationStatus": random.choice([0, 1]),
        },
    }


uri = f"ws://{os.getenv('IP4_ADDRESS')}:{os.getenv('PORT')}/ws/smart-hydroponic/device"


async def send_data(uri, data):
    async with websockets.connect(uri) as websocket:
        await websocket.send(data)


async def receive_data(uri):
    async with websockets.connect(uri) as websocket:
        data = await websocket.recv()
        print(json.loads(data))


async def send_pong(uri):
    async with websockets.connect(uri) as websocket:
        await websocket.pong()


async def main():
    while True:
        await send_data(uri, json.dumps(data_plant()))
        # await asyncio.sleep(5)
        await send_data(uri, json.dumps(data_environment()))
        # await asyncio.sleep(7)
        await send_data(uri, json.dumps(data_actuator()))
        await asyncio.sleep(5)

        await receive_data(uri)
        await asyncio.sleep(5)


asyncio.get_event_loop().run_until_complete(main())
