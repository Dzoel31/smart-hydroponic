import json
import time
import random
import dotenv
import os
import requests
import asyncio

dotenv.load_dotenv()

# Constants from Arduino code
MOISTURE_THRESHOLD = 65.0
TEMPERATURE_THRESHOLD = 30.0
DEVICE_ID = "esp8266-actuator-device"
DATA_SEND_INTERVAL = 5  # seconds
FETCH_DATA_INTERVAL = 2  # seconds
RECONNECT_DELAY = 5  # seconds between reconnection attempts

# Base URL for RESTful API
base_url = f"http://{os.getenv('HOST')}:{os.getenv('PORT')}/api/smart-hydroponic/v1/actuators"
# URL for fetching data (automation mode)
fetch_url_auto_sensor = "http://localhost:15000/api/smart-hydroponic/v1/sensor/latest"
fetch_url_auto_environment = "http://localhost:15000/api/smart-hydroponic/v1/environment/latest"

# Simulated pin states
pins = {
    "RELAY_PUMP_1": False,   # False = HIGH (off), True = LOW (on)
    "RELAY_PUMP_2": False,
    "RELAY_LIGHT_1": False,
    "RELAY_LIGHT_2": False
}

# State variables
actuator_state = {
    "pumpStatus": 0,
    "lightStatus": 0,
    "automationStatus": 1,
    "isActuatorConnected": True  # Assume device is always connected
}

def update_outputs():
    """Updates the simulated pin states based on actuator status"""
    # LOW activates relay, HIGH deactivates it
    pins["RELAY_PUMP_1"] = actuator_state["pumpStatus"] == 1
    pins["RELAY_PUMP_2"] = actuator_state["pumpStatus"] == 1
    pins["RELAY_LIGHT_1"] = actuator_state["lightStatus"] == 1
    pins["RELAY_LIGHT_2"] = actuator_state["lightStatus"] == 1
    
    print(f"Pump status: {'ON' if actuator_state['pumpStatus'] == 1 else 'OFF'}")
    print(f"Light status: {'ON' if actuator_state['lightStatus'] == 1 else 'OFF'}")

def handle_automatic_mode():
    """Handles automatic mode logic"""
    print("Operating in automatic mode")

    response_sensor = requests.get(fetch_url_auto_sensor)
    response_environment = requests.get(fetch_url_auto_environment)

    if response_sensor.status_code != 200 or response_environment.status_code != 200:
        print("Failed to fetch sensor or environment data")
        return

    data_sensor = response_sensor.json()
    data_environment = response_environment.json()
    data = {**data_sensor['data'][0], **data_environment['data'][0]}  # Combine sensor and environment data
    # Pump control based on moisture
    if "moistureavg" in data and data["moistureavg"] is not None:
        moistureAvg = data["moistureavg"]
        actuator_state["pumpStatus"] = 1 if moistureAvg < MOISTURE_THRESHOLD else 0
    
    # Light control based on temperature
    if "avg_temperature" in data and data["avg_temperature"] is not None:
        temperatureAvg = data["avg_temperature"]
        actuator_state["lightStatus"] = 1 if temperatureAvg < TEMPERATURE_THRESHOLD else 0

def handle_manual_mode(data):
    """Handles manual mode logic"""
    print("Operating in manual mode")

    if "pump_status" in data:
        actuator_state["pumpStatus"] = data["pump_status"]

    if "light_status" in data:
        actuator_state["lightStatus"] = data["light_status"]

def send_status_update():
    """Sends status update to the server"""
    try:
        status_data = {
            "device_id": DEVICE_ID,
            "type": "update_data",
            "data": {
                "pumpStatus": actuator_state["pumpStatus"],
                "lightStatus": actuator_state["lightStatus"],
                "automationStatus": actuator_state["automationStatus"]
            }
        }
        
        response = requests.post(base_url, json=status_data)
        if response.status_code == 201:
            print("Status created successfully.")
            return True
        response.raise_for_status()
        print(f"Status sent: {response.json()}")
        return True
    except Exception as e:
        print(f"Failed to send status update: {e}")
        return False

def fetch_data():
    """Fetches data from the server"""
    try:
        response = requests.get("http://localhost:15000/api/smart-hydroponic/v1/actuator/latest")
        response.raise_for_status()
        data = response.json()
        print(f"Received: {data}")
        return data
    except Exception as e:
        print(f"Failed to fetch data: {e}")
        return None

def device_loop():
    """Main device loop with separate intervals for fetching and sending data"""
    last_send_time = time.time()
    last_fetch_time = time.time()
    while True:
        try:
            while True:
                current_time = time.time()

                # Fetch data at FETCH_DATA_INTERVAL
                if current_time - last_fetch_time >= FETCH_DATA_INTERVAL:
                    data = fetch_data()
                    if data:
                        # Check for automation status update
                        if "automation_status" in data['data'][0]:
                            actuator_state["automationStatus"] = data["data"][0]["automation_status"]
                            print(f"Automation status: {actuator_state['automationStatus']}")

                        # Handle according to automation mode
                        if actuator_state["automationStatus"] == 1:
                            handle_automatic_mode()
                        else:
                            handle_manual_mode(data['data'][0])

                        update_outputs()
                    last_fetch_time = current_time

                # Send status updates at DATA_SEND_INTERVAL
                if current_time - last_send_time >= DATA_SEND_INTERVAL:
                    success = send_status_update()
                    if not success:
                        break
                    last_send_time = current_time

                # Simulate some hardware delay
                time.sleep(0.1)

        except Exception as e:
            print(f"Connection error: {e}")

        finally:
            print(f"Disconnected. Reconnecting in {RECONNECT_DELAY} seconds...")
            time.sleep(RECONNECT_DELAY)

def main():
    print("Starting ESP8266 environment simulator...")
    device_loop()

if __name__ == "__main__":
    main()
