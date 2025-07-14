import requests
import random
import dotenv
import os
import time
import json

dotenv.load_dotenv()

DEVICE_ID = "esp32-plant-device"
BASE_URL = f"http://{os.getenv('HOST')}:{os.getenv('PORT')}/api/smart-hydroponic/v1/sensors"

def data_plant():
    return {
        "device_id": DEVICE_ID,
        "type": "update_data",
        "data": {
            "moisture1": 60,
            "moisture2": 60,
            "moisture3": 60,
            "moisture4": 50,
            "moisture5": 60,
            "moisture6": 60,
            "flowrate": random.randint(1, 100),
            "total_litres": random.randint(1, 100),
            "distance_cm": random.randint(1, 100),
            "ph": random.uniform(5.5, 7.5),
            "tds": random.randint(100, 1000),
        },
    }

def send_data():
    while True:
        try:
            data = data_plant()
            response = requests.post(BASE_URL, json=data)
            if response.status_code == 201:
                print(f"Sent data: {json.dumps(data)}\nResponse: {response.json()}")
            else:
                print(f"Failed to send data. Status code: {response.status_code}, Response: {response.text}")
            time.sleep(5)  # Delay of 5 seconds before sending the next data
        except requests.exceptions.RequestException as e:
            print(f"Connection error: {e}. Retrying in 5 seconds...")
            time.sleep(5)

if __name__ == "__main__":
    send_data()
