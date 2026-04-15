import json
import time
import asyncio
import websockets
import dotenv

dotenv.load_dotenv()

# Constants from Arduino code
MOISTURE_THRESHOLD = 65.0
TEMPERATURE_THRESHOLD = 30.0
# DEVICE_ID = "esp8266-actuator-device"
DATA_SEND_INTERVAL = 5  # seconds
RECONNECT_DELAY = 5  # seconds between reconnection attempts

# uri = f"ws://{os.getenv('HOST')}/ws/smart-hydroponic/device"
uri = "ws://localhost:8000/hydroponics/ws/actuator-data"

# Simulated pin states
pins = {
    "RELAY_PUMP_1": False,  # False = HIGH (off), True = LOW (on)
    "RELAY_PUMP_2": False,
    "RELAY_LIGHT_1": False,
    "RELAY_LIGHT_2": False,
}

# State variables
actuator_state = {
    "pumpStatus": 0,
    "lightStatus": 0,
    "automationStatus": 1,
    "isActuatorConnected": False,
}


async def connect_to_websocket():
    """Simulates connecting to the WebSocket server"""
    try:
        websocket = await websockets.connect(uri)
        actuator_state["isActuatorConnected"] = True
        print("Connected to WebSocket server")
        return websocket
    except Exception as e:
        print(f"Failed to connect to WebSocket server: {e}")
        actuator_state["isActuatorConnected"] = False
        return None


# async def register_device(websocket):
#     """Registers the device with the server"""
#     try:
#         rooms = ["command", "plant", "environment"]
#         for room in rooms:
#             register_data = {
#                 "deviceId": DEVICE_ID,
#                 "type": "join",
#                 "room": room,
#             }
#             await websocket.send(json.dumps(register_data))
#             print(f"Device registered in {room}: {json.dumps(register_data)}")
#         return True
#     except Exception as e:
#         print(f"Failed to register device: {e}")
#         return False


def update_outputs():
    """Updates the simulated pin states based on actuator status"""
    # LOW activates relay, HIGH deactivates it
    pins["RELAY_PUMP_1"] = actuator_state["pumpStatus"] == 1
    pins["RELAY_PUMP_2"] = actuator_state["pumpStatus"] == 1
    pins["RELAY_LIGHT_1"] = actuator_state["lightStatus"] == 1
    pins["RELAY_LIGHT_2"] = actuator_state["lightStatus"] == 1

    print(f"Pump status: {'ON' if actuator_state['pumpStatus'] == 1 else 'OFF'}")
    print(f"Light status: {'ON' if actuator_state['lightStatus'] == 1 else 'OFF'}")


def handle_automatic_mode(message_data):
    """Handles automatic mode logic"""
    print("Operating in automatic mode")

    # Pump control based on moisture
    if "moisture_avg" in message_data and message_data["moisture_avg"] is not None:
        moistureAvg = message_data["moisture_avg"]
        actuator_state["pumpStatus"] = 1 if moistureAvg < MOISTURE_THRESHOLD else 0

    # Light control based on temperature
    if (
        "temperature_avg" in message_data
        and message_data["temperature_avg"] is not None
    ):
        temperatureAvg = message_data["temperature_avg"]
        actuator_state["lightStatus"] = (
            1 if temperatureAvg < TEMPERATURE_THRESHOLD else 0
        )


def handle_manual_mode(message_data):
    """Handles manual mode logic"""
    print("Operating in manual mode")

    if "pump_status" in message_data:
        actuator_state["pumpStatus"] = message_data["pump_status"]

    if "light_status" in message_data:
        actuator_state["lightStatus"] = message_data["light_status"]


async def send_status_update(websocket):
    """Sends status update to the server"""
    if not websocket or not actuator_state["isActuatorConnected"]:
        return False

    try:
        status_data = {
            "pump_status": actuator_state["pumpStatus"],
            "light_status": actuator_state["lightStatus"],
            "automation_status": actuator_state["automationStatus"],
        }

        await websocket.send(json.dumps(status_data))
        print(f"Status sent: {json.dumps(status_data)}")
        return True
    except Exception as e:
        print(f"Failed to send status update: {e}")
        actuator_state["isActuatorConnected"] = False
        return False


async def handle_message(message):
    """Processes incoming WebSocket messages"""
    try:
        message_data = json.loads(message)
        print(f"Received: {message}")

        # if "data" not in message_data:
        #     print("No data field in message")
        #     return

        # Check for automation status update
        if "automation_status" in message_data:
            actuator_state["automationStatus"] = message_data["automation_status"]
            print(f"Automation status: {actuator_state['automationStatus']}")

        # Handle according to automation mode
        if actuator_state["automationStatus"] == 1:
            handle_automatic_mode(message_data)
        else:
            handle_manual_mode(message_data)

        update_outputs()
    except json.JSONDecodeError:
        print("Invalid JSON received")


async def device_loop():
    """Main device loop with connection handling"""
    while True:
        websocket = None
        try:
            # Connect to WebSocket
            websocket = await connect_to_websocket()

            # If connection successful, register device
            if websocket:
                # registered = await register_device(websocket)
                # if not registered:
                #     raise Exception("Failed to register device")

                # Main operation loop
                last_send_time = time.time()

                while True:
                    # Poll for messages (non-blocking)
                    try:
                        message = await asyncio.wait_for(websocket.recv(), timeout=5.0)
                        await handle_message(message)
                    except asyncio.TimeoutError:
                        pass
                    except Exception as e:
                        print(f"Error receiving message: {e}")
                        break

                    # Send status updates periodically
                    current_time = time.time()
                    if current_time - last_send_time >= DATA_SEND_INTERVAL:
                        success = await send_status_update(websocket)
                        if not success:
                            break
                        last_send_time = current_time

                    # Simulate some hardware delay
                    await asyncio.sleep(0.1)

        except Exception as e:
            print(f"Connection error: {e}")

        finally:
            # Clean up before trying to reconnect
            if websocket:
                try:
                    await websocket.close()
                except Exception as e:
                    print(f"Error closing WebSocket: {e}")

            actuator_state["isActuatorConnected"] = False
            print(f"Disconnected. Reconnecting in {RECONNECT_DELAY} seconds...")
            await asyncio.sleep(RECONNECT_DELAY)


async def main():
    print("Starting ESP8266 environment simulator...")
    await device_loop()


if __name__ == "__main__":
    asyncio.run(main())
