import requests
import random
import dotenv
import os
import time
import json

dotenv.load_dotenv()

DEVICE_ID = "esp32-environment-device"
url = f"http://{os.getenv('HOST')}:{os.getenv('PORT')}/api/smart-hydroponic/v1/environments"

def data_environment():
    return {
        "device_id": DEVICE_ID,
        "type": "update_data",
        "data": {
            "temperatureAtas": 20,
            "temperatureBawah": 20,
            "humidityAtas": random.randint(1, 100),
            "humidityBawah": random.randint(1, 100),
        },
    }

def send_data():
    while True:
        try:
            data = data_environment()
            response = requests.post(url, json=data)
            if response.status_code == 201:
                print(f"Sent data: {json.dumps(data)}\nResponse: {response.json()}")
            else:
                print(f"Failed to send data: {response.status_code}, {response.text}")
            time.sleep(5)  # Delay of 5 seconds before sending the next data
        except requests.exceptions.RequestException as e:
            print(f"Connection error: {e}. Retrying in 5 seconds...")
            time.sleep(5)

if __name__ == "__main__":
    send_data()
