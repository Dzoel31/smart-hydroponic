#include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFi.h>
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

// State variables
bool isActuatorConnected = false;
int pumpStatus = 0;
int lightStatus = 0;
int automationStatus = 0;
int moistureLevel = 0;
int seq = 1;
unsigned long send_time = 0;
int last_seq_sent = 0;

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
void onMessageCallback(WebsocketsMessage message);
void sendStatusUpdate();
void sendActuatorAck(const char *messageType, const char *messageId);
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

    connectToWifi();
    setupWebSocket();
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

    if (doc.containsKey("status") && String(doc["status"].as<const char *>()) == "ack")
    {
        int ackSeq = doc.containsKey("seq") ? doc["seq"].as<int>() : last_seq_sent;
        unsigned long latency = millis() - send_time;
        Serial.printf("[METRIC] Seq: %d | Latency: %lu ms\n", ackSeq, latency);
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
    if (dataToProcess.containsKey("avg_temperature"))
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

    if (doc.containsKey("command_id"))
    {
        sendActuatorAck("dashboard_control", doc["command_id"].as<const char *>());
    }
    else if (doc.containsKey("correlation_id"))
    {
        sendActuatorAck("inter_node_forward", doc["correlation_id"].as<const char *>());
    }
}

void handleAutomaticMode(JsonVariant data)
{
    Serial.println("Operating in automatic mode");

    // Get moisture and temperature values
    float moistureAvg = data.containsKey("moisture_avg") ? data["moisture_avg"].as<float>() : lastMoistureAvg;
    float temperatureAvg = data.containsKey("avg_temperature") ? data["avg_temperature"].as<float>() : lastTemperatureAvg;

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

    jsonDoc["seq"] = seq;
    jsonDoc["pump_status"] = pumpStatus;
    jsonDoc["light_status"] = lightStatus;
    jsonDoc["automation_status"] = automationStatus;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    if (clientActuator.available())
    {
        last_seq_sent = seq;
        send_time = millis();
        clientActuator.send(jsonString);
        seq++;
        Serial.println("Status sent: " + jsonString);
    }
}

void sendActuatorAck(const char *messageType, const char *messageId)
{
    StaticJsonDocument<256> ackDoc;
    ackDoc["type"] = "actuator_ack";
    ackDoc["ack_type"] = messageType;
    ackDoc["command_id"] = messageId;
    ackDoc["correlation_id"] = messageId;
    ackDoc["pump_status"] = pumpStatus;
    ackDoc["light_status"] = lightStatus;
    ackDoc["automation_status"] = automationStatus;
    ackDoc["device_id"] = DEVICE_ID;
    ackDoc["ack_time_ms"] = millis();

    String ackString;
    serializeJson(ackDoc, ackString);

    if (clientActuator.available())
    {
        clientActuator.send(ackString);
        Serial.println("[ACTUATOR_ACK] " + ackString);
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
