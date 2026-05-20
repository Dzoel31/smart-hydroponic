#include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFi.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

// Pin Definitions
#define RELAY_PUMP_1 4   // D2
#define RELAY_PUMP_2 5   // D1
#define RELAY_LIGHT_1 12 // D6
#define RELAY_LIGHT_2 14 // D5

// Configuration
const char *WIFI_SSID = "FIK-Hotspot";
const char *WIFI_PASSWORD = "T4nahairku";
// const char *WEBSOCKET_URL = "ws://103.147.92.179";
const char *WEBSOCKET_URL = "ws://103.147.92.179/smart-hydroponic/api/v2/hydroponics/ws/actuator-data";
const char *DEVICE_ID = "esp8266-actuator-device";
const unsigned long DATA_SEND_INTERVAL = 5000;      // 5 seconds
const unsigned long WIFI_RECONNECT_TIMEOUT = 10000; // 10 seconds
const float MOISTURE_THRESHOLD = 60;
const float TEMPERATURE_THRESHOLD = 30.0;

const uint32_t STATE_MAGIC = 0x48594C31;
const int EEPROM_SIZE = 64;

struct PersistedState
{
    uint32_t magic;
    uint8_t automationStatus;
    uint8_t pumpStatus;
    uint8_t lightStatus;
    float lastMoistureAvg;
    float lastTemperatureAvg;
};

// State variables
bool isActuatorConnected = false;
int pumpStatus = 0;
int lightStatus = 0;
int automationStatus = 0;
int moistureLevel = 0;

float lastMoistureAvg = 0;
float lastTemperatureAvg = 0;

using namespace websockets;
WebsocketsClient clientActuator;
String registerString;

// Function prototypes
void connectToWifi();
void setupWebSocket();
void registerDevice();
void handleAutomaticMode(JsonVariant data);
void handleManualMode(JsonVariant data);
void updateOutputs();
void loadPersistedState();
void savePersistedState();
void onMessageCallback(WebsocketsMessage message);
void sendStatusUpdate();
void checkConnections();

void setup()
{
    Serial.begin(115200);

    // Initialize output pins
    pinMode(RELAY_PUMP_1, OUTPUT);
    pinMode(RELAY_PUMP_2, OUTPUT);
    pinMode(RELAY_LIGHT_1, OUTPUT);
    pinMode(RELAY_LIGHT_2, OUTPUT);

    // Default state for relays (LOW is ON for most relay modules)
    digitalWrite(RELAY_PUMP_1, HIGH);  // OFF initially
    digitalWrite(RELAY_PUMP_2, HIGH);  // OFF initially
    digitalWrite(RELAY_LIGHT_1, HIGH); // OFF initially
    digitalWrite(RELAY_LIGHT_2, HIGH); // OFF initially

    EEPROM.begin(EEPROM_SIZE);
    loadPersistedState();
    updateOutputs();

    connectToWifi();
    setupWebSocket();
}

void loadPersistedState()
{
    PersistedState storedState;
    EEPROM.get(0, storedState);

    if (storedState.magic != STATE_MAGIC)
    {
        automationStatus = 0;
        pumpStatus = 0;
        lightStatus = 0;
        lastMoistureAvg = 0;
        lastTemperatureAvg = 0;
        savePersistedState();
        Serial.println("No persisted actuator state found, using safe defaults.");
        return;
    }

    automationStatus = storedState.automationStatus;
    pumpStatus = storedState.pumpStatus;
    lightStatus = storedState.lightStatus;
    lastMoistureAvg = storedState.lastMoistureAvg;
    lastTemperatureAvg = storedState.lastTemperatureAvg;

    Serial.println("Restored actuator state from EEPROM.");
    Serial.println("Automation status: " + String(automationStatus));
    Serial.println("Pump status: " + String(pumpStatus));
    Serial.println("Light status: " + String(lightStatus));
}

void savePersistedState()
{
    PersistedState storedState;
    storedState.magic = STATE_MAGIC;
    storedState.automationStatus = automationStatus;
    storedState.pumpStatus = pumpStatus;
    storedState.lightStatus = lightStatus;
    storedState.lastMoistureAvg = lastMoistureAvg;
    storedState.lastTemperatureAvg = lastTemperatureAvg;

    EEPROM.put(0, storedState);
    EEPROM.commit();
}

void registerDevice()
{
    StaticJsonDocument<256> registerJson;
    registerJson["physical_id"] = DEVICE_ID;

    serializeJson(registerJson, registerString);

    clientActuator.send(registerString);
    Serial.println("Device registered: " + registerString);
}

void loop()
{
    clientActuator.poll();
    checkConnections();

    // Send status update periodically
    static unsigned long lastSendTime = 0;
    if (clientActuator.available())
    {
        if (millis() - lastSendTime >= DATA_SEND_INTERVAL)
        {
            sendStatusUpdate();
            lastSendTime = millis();
        }
    }
    else
    {
        Serial.println("WebSocket not available");
        setupWebSocket();
    }
}

void connectToWifi()
{
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(3000);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
    Serial.println("IP: " + WiFi.localIP().toString());
}

void setupWebSocket()
{
    if (clientActuator.connect(WEBSOCKET_URL))
    {
        Serial.println("Connected to WebSocket server");
        clientActuator.onMessage(onMessageCallback);
        registerDevice();
        clientActuator.send(registerString);
        Serial.println("Device registered: " + registerString);
        isActuatorConnected = true;
    }
    else
    {
        Serial.println("Failed to connect to WebSocket server");
        isActuatorConnected = false;
    }
}

void onMessageCallback(WebsocketsMessage message)
{
    String command = message.data();
    Serial.println("Received: " + command);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, command);

    if (error)
    {
        Serial.println("JSON parsing failed!");
        return;
    }

    if (doc.isNull())
    {
        Serial.println("No data field in message");
        return;
    }

    // NEW: Target the "payload" object if it exists, otherwise fallback to root
    JsonVariant dataToProcess;
    if (doc.containsKey("payload"))
    {
        dataToProcess = doc["payload"];
    }
    else
    {
        dataToProcess = doc;
    }

    // Now check for status updates inside dataToProcess
    if (dataToProcess.containsKey("automation_status"))
    {
        automationStatus = dataToProcess["automation_status"].as<int>();
        Serial.println("Automation status: " + String(automationStatus));
    }
    if (dataToProcess.containsKey("moisture_avg"))
    {
        lastMoistureAvg = dataToProcess["moisture_avg"].as<float>();
        Serial.println("Moisture Avg: " + String(lastMoistureAvg));
    }
    if (dataToProcess.containsKey("temperature_avg"))
    {
        lastTemperatureAvg = dataToProcess["temperature_avg"].as<float>();
        Serial.println("Temperature Avg: " + String(lastTemperatureAvg));
    }
    else if (dataToProcess.containsKey("avg_temperature"))
    {
        lastTemperatureAvg = dataToProcess["avg_temperature"].as<float>();
        Serial.println("Temperature Avg: " + String(lastTemperatureAvg));
    }

    // Handle according to automation mode
    if (automationStatus == 1)
    {
        handleAutomaticMode(dataToProcess);
    }
    else
    {
        handleManualMode(dataToProcess);
    }

    updateOutputs();

    savePersistedState();
}

void handleAutomaticMode(JsonVariant data)
{
    Serial.println("Operating in automatic mode");

    // Get moisture and temperature values
    float moistureAvg = data.containsKey("moisture_avg") ? data["moisture_avg"].as<float>() : lastMoistureAvg;
    float temperatureAvg = data.containsKey("temperature_avg") ? data["temperature_avg"].as<float>() : (data.containsKey("avg_temperature") ? data["avg_temperature"].as<float>() : lastTemperatureAvg);

    // Only update if values are valid
    if (!isnan(moistureAvg))
    {
        pumpStatus = (moistureAvg < MOISTURE_THRESHOLD) ? 1 : 0;
        Serial.println("Moisture: " + String(moistureAvg) + " -> Pump: " + String(pumpStatus));
    }

    if (!isnan(temperatureAvg))
    {
        lightStatus = (temperatureAvg < TEMPERATURE_THRESHOLD) ? 1 : 0;
        Serial.println("Temperature: " + String(temperatureAvg) + " -> Light: " + String(lightStatus));
    }
}

void handleManualMode(JsonVariant data)
{
    Serial.println("Operating in manual mode");

    if (data.containsKey("pump_status"))
    {
        pumpStatus = data["pump_status"].as<int>();
    }

    if (data.containsKey("light_status"))
    {
        lightStatus = data["light_status"].as<int>();
    }
}

void updateOutputs()
{
    // LOW activates the relay, HIGH deactivates it
    digitalWrite(RELAY_PUMP_1, pumpStatus ? LOW : HIGH);
    digitalWrite(RELAY_PUMP_2, pumpStatus ? LOW : HIGH);
    digitalWrite(RELAY_LIGHT_1, lightStatus ? LOW : HIGH);
    digitalWrite(RELAY_LIGHT_2, lightStatus ? LOW : HIGH);
}

void sendStatusUpdate()
{
    StaticJsonDocument<256> jsonDoc;

    jsonDoc["pump_status"] = pumpStatus;
    jsonDoc["light_status"] = lightStatus;
    jsonDoc["automation_status"] = automationStatus;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    if (clientActuator.available())
    {
        clientActuator.send(jsonString);
        Serial.println("Status sent: " + jsonString);
    }
}

void checkConnections()
{
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi disconnected, reconnecting...");
        connectToWifi();
    }

    // Check WebSocket connection
    if (!clientActuator.available() && !isActuatorConnected)
    {
        Serial.println("WebSocket disconnected, reconnecting...");
        setupWebSocket();
    }
}
