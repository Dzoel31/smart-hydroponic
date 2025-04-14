#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "DHT.h"

// Pin and sensor definitions
#define TDS_SENSOR_PIN A0
#define VREF 3.3
#define SCOUNT 30
#define DHT11_PIN 14
#define DHT22_PIN 26
#define DHT11_TYPE DHT11
#define DHT22_TYPE DHT22

// Network configuration
const char *ssid = "Podcast Area";
const char *password = "iriunwebcam";
const char *websocket_server = "ws://103.147.92.179:15000/ws/smart-hydroponic/device";
const char *device_id = "esp32-environment-device";

// Timing constants
const unsigned long SEND_INTERVAL = 5000; // 10 seconds
const unsigned long WIFI_TIMEOUT = 10000;  // 10 seconds
const unsigned long TDS_SAMPLE_INTERVAL = 40; // 40 milliseconds

// Global variables
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
unsigned long lastSendTime = 0;
unsigned long lastAnalogSampleTime = 0;

// Sensor readings
float tdsValue = 0;
float temperature_atas = 0;
float humidity_atas = 0;
float temperature_bawah = 0;
float humidity_bawah = 0;

// Objects
using namespace websockets;
WebsocketsClient client;
DHT dht11(DHT11_PIN, DHT11_TYPE);
DHT dht22(DHT22_PIN, DHT22_TYPE);

// Function prototypes
int getMedianNum(int bArray[], int iFilterLen);
void checkWiFiConnection();
bool connectToWebSocket();
void registerDevice();
void readDHTSensors();
void readTDSSensor();
void sendSensorData();

void setup() {
  Serial.begin(115200);
  pinMode(TDS_SENSOR_PIN, INPUT);
  
  // Initialize sensors
  dht11.begin();
  dht22.begin();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println("\nConnected to WiFi");
  
  // Connect to WebSocket server and register device
  if (connectToWebSocket()) {
    registerDevice();
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Sample TDS sensor at regular intervals
  if (currentMillis - lastAnalogSampleTime > TDS_SAMPLE_INTERVAL) {
    lastAnalogSampleTime = currentMillis;
    analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
  }
  
  // Read DHT sensors
  readDHTSensors();
  
  // Read TDS sensor
  readTDSSensor();
  
  // Send data at regular intervals
  if (currentMillis - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = currentMillis;
    sendSensorData();
  }
  
  // Maintain connections
  checkWiFiConnection();
  client.poll();
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
    
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  return (iFilterLen & 1) ? bTab[(iFilterLen - 1) / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
      Serial.println("Reconnecting...");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected to WiFi!");
    } else {
      Serial.println("Failed to reconnect. Will retry later.");
    }
  }
}

bool connectToWebSocket() {
  bool connected = client.connect(websocket_server);
  if (connected) {
    Serial.println("Connected to WebSocket Server");
    return true;
  } else {
    Serial.println("Failed to connect to WebSocket Server");
    return false;
  }
}

void registerDevice() {
  StaticJsonDocument<256> registerJson;
  registerJson["device_id"] = device_id;
  registerJson["type"] = "register";

  String registerString;
  serializeJson(registerJson, registerString);

  Serial.println("Registering device: " + registerString);
  client.send(registerString);
}

void readDHTSensors() {
  temperature_atas = dht11.readTemperature();
  humidity_atas = dht11.readHumidity();
  temperature_bawah = dht22.readTemperature();
  humidity_bawah = dht22.readHumidity();

  if (isnan(temperature_atas) || isnan(humidity_atas) || isnan(temperature_bawah) || isnan(humidity_bawah)) {
    Serial.println("Failed to read from DHT sensors!");
  }
}

void readTDSSensor() {
  // Copy buffer for median calculation
  for (int i = 0; i < SCOUNT; i++) {
    analogBufferTemp[i] = analogBuffer[i];
  }
  
  float averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * VREF / 4095.0;
  float compensationCoefficient = 1.0 + 0.02 * (temperature_bawah - 25.0); // Using temperature_bawah for compensation
  float compensationVoltage = averageVoltage / compensationCoefficient;
  tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage);
}

void sendSensorData() {
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["device_id"] = device_id;

  JsonObject data = jsonDoc.createNestedObject("data");
  data["temperature_atas"] = temperature_atas;
  data["humidity_atas"] = humidity_atas;
  data["temperature_bawah"] = temperature_bawah;
  data["humidity_bawah"] = humidity_bawah;
  data["tdsValue"] = tdsValue;

  String dataStr;
  serializeJson(jsonDoc, dataStr);

  if (client.available()) {
    client.send(dataStr);
    Serial.println("Sent: " + dataStr);
  } else {
    Serial.println("WebSocket client not available, attempting to reconnect...");
    if (connectToWebSocket()) {
      registerDevice();
      // Try to send again after reconnection
      client.send(dataStr);
      Serial.println("Sent after reconnection: " + dataStr);
    }
  }
}
