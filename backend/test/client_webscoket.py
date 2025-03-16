import websockets
import random
import dotenv
import os
import asyncio
import json

dotenv.load_dotenv()


def data_plant():
    return {
        "moisture1": random.randint(0, 100),
        "moisture2": random.randint(0, 100),
        "moisture3": random.randint(0, 100),
        "moisture4": random.randint(0, 100),
        "moisture5": random.randint(0, 100),
        "moisture6": random.randint(0, 100),
        "moistureAvg": 50,
        "flowRate": random.randint(0, 100),
        "totalLitres": random.randint(0, 100),
        "distanceCm": random.randint(0, 100),
    }

def data_environment():
    return {
        "temperature_atas": 20,
        "temperature_bawah": 30,
        "humidity_atas": random.randint(0, 100),
        "humidity_bawah": random.randint(0, 100),
        "tdsValue": random.randint(0, 100),
    }

def data_actuator():
    return {
        "pumpStatus": random.choice([0, 1]),
        "lightStatus": random.choice([0, 1]),
        "otomationStatus": random.choice([0, 1]),
    }

plant_data_uri = f"ws://{os.getenv('IP4_ADDRESS')}:{os.getenv('PORT_WS')}/plantdata"
environment_data_uri = f"ws://{os.getenv('IP4_ADDRESS')}:{os.getenv('PORT_WS')}/environmentdata"
actuator_data_uri = f"ws://{os.getenv('IP4_ADDRESS')}:{os.getenv('PORT_WS')}/actuator"

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
        await send_data(plant_data_uri, json.dumps(data_plant()))
        await send_data(environment_data_uri, json.dumps(data_environment()))
        # await send_data(actuator_data_uri, json.dumps(data_actuator()))
        await asyncio.sleep(10)

asyncio.get_event_loop().run_until_complete(main())