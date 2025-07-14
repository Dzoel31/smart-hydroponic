#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

// Pin Definitions
#define RELAY_PUMP_1  4   // D2
#define RELAY_PUMP_2  5   // D1
#define RELAY_LIGHT_1 12  // D6
#define RELAY_LIGHT_2 14  // D5

// Configuration
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";
const char *API_URL = "";
const char *ENDPOINT_AUTOMATIC_MODE_ENV = "";
const char *ENDPOINT_AUTOMATIC_MODE_SENSOR = "";
const char *FETCH_ENDPOINT_MANUAL_MODE = "";
const char* DEVICE_ID = "esp8266-actuator-device";
const unsigned long DATA_FETCH_INTERVAL = 2000; // 2 seconds
const unsigned long DATA_SEND_INTERVAL = 5000; // 5 seconds
const unsigned long WIFI_RECONNECT_TIMEOUT = 10000; // 10 seconds
const float MOISTURE_THRESHOLD = 65;
const float TEMPERATURE_THRESHOLD = 30.0;

// State variables
int pumpStatus = 0;
int lightStatus = 0;
int automationStatus = 0;
float lastMoistureAvg = 0;
float lastTemperatureAvg = 0;

void connectToWifi();
void sendStatusUpdate();
void fetchActuatorData();
void handleAutomaticMode(float moistureAvg, float temperatureAvg);
void handleManualMode(JsonVariant data);
void updateOutputs();
bool ensureWifiConnection();
void initializeRelays();

void setup() {
  Serial.begin(115200);
  initializeRelays();
  connectToWifi();
}

void loop() {
  static unsigned long lastSendTime = 0;
  static unsigned long lastFetchTime = 0;

  if (millis() - lastSendTime >= DATA_SEND_INTERVAL) {
    sendStatusUpdate();
    lastSendTime = millis();
  }

  if (millis() - lastFetchTime >= DATA_SEND_INTERVAL) {
    fetchActuatorData();
    lastFetchTime = millis();
  }
}

void connectToWifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
  Serial.println("IP: " + WiFi.localIP().toString());
}

bool ensureWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    connectToWifi();
    return false;
  }
  return true;
}

void sendStatusUpdate() {
  if (!ensureWifiConnection()) return;

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> jsonDoc;
  jsonDoc["device_id"] = DEVICE_ID;
  jsonDoc["type"] = "update_data";

  JsonObject dataObj = jsonDoc.createNestedObject("data");
  dataObj["pumpStatus"] = pumpStatus;
  dataObj["lightStatus"] = lightStatus;
  dataObj["automationStatus"] = automationStatus;

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  int httpResponseCode = http.POST(jsonString);
  if (httpResponseCode > 0) {
    Serial.println("Status sent: " + jsonString);
    Serial.println("Response code: " + String(httpResponseCode));
  } else {
    Serial.println("Failed to send status update");
  }

  http.end();
}

void fetchActuatorData() {
  if (!ensureWifiConnection()) return;

  // Step 1: Fetch manual endpoint to get automationStatus and manual data
  HTTPClient http;
  http.begin(FETCH_ENDPOINT_MANUAL_MODE);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("Received (manual): " + payload);

    StaticJsonDocument<256> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, payload);

    if (error) {
      Serial.println("JSON parsing failed (manual)!");
      http.end();
      return;
    }

    JsonArray dataArr = jsonDoc["data"];
    if (dataArr.isNull() || dataArr.size() == 0) {
      Serial.println("No data field or empty array in manual response");
      http.end();
      return;
    }
    JsonObject data = dataArr[0];

    // Selalu update automationStatus
    automationStatus = data["automation_status"].as<int>();

    if (automationStatus == 1) {
      handleAutomaticMode();
    } else {
      handleManualMode(data);
    }
    updateOutputs();
  } else {
    Serial.println("Failed to fetch manual mode data");
  }
  http.end();
}

void handleAutomaticMode() {
  Serial.println("Operating in automatic mode");

  HTTPClient httpEnv;
  httpEnv.begin(ENDPOINT_AUTOMATIC_MODE_ENV);
  int httpResponseCode = httpEnv.GET();
  HTTPClient httpSensor;
  httpSensor.begin(ENDPOINT_AUTOMATIC_MODE_SENSOR);
  int httpResponseCodeSensor = httpSensor.GET();

  if (httpResponseCode > 0 && httpResponseCodeSensor > 0) {
    String payloadEnv = httpEnv.getString();
    String payloadSensor = httpSensor.getString();

    Serial.println("Received (automatic env): " + payloadEnv);
    Serial.println("Received (automatic sensor): " + payloadSensor);

    StaticJsonDocument<256> jsonDocEnv;
    StaticJsonDocument<256> jsonDocSensor;

    DeserializationError errorEnv = deserializeJson(jsonDocEnv, payloadEnv);
    DeserializationError errorSensor = deserializeJson(jsonDocSensor, payloadSensor);

    if (errorEnv || errorSensor) {
      Serial.println("JSON parsing failed for automatic mode!");
      return;
    }

    JsonArray arrSensor = jsonDocSensor["data"];
    JsonArray arrEnv = jsonDocEnv["data"];
    if (arrSensor.isNull() || arrSensor.size() == 0 || arrEnv.isNull() || arrEnv.size() == 0) {
      Serial.println("No data or empty array in automatic mode response");
      return;
    }
    JsonObject objSensor = arrSensor[0];
    JsonObject objEnv = arrEnv[0];

    float moistureAvg = objSensor["moistureavg"].as<float>();
    float temperatureAvg = objEnv["avg_temperature"].as<float>();

    pumpStatus = (moistureAvg < MOISTURE_THRESHOLD) ? 1 : 0;
    lightStatus = (temperatureAvg < TEMPERATURE_THRESHOLD) ? 1 : 0;

    Serial.println("Moisture: " + String(moistureAvg) + " -> Pump: " + String(pumpStatus));
    Serial.println("Temperature: " + String(temperatureAvg) + " -> Light: " + String(lightStatus));
  } else {
    Serial.println("Failed to fetch automatic mode data");
  }
}

void handleManualMode(JsonVariant data) {
  Serial.println("Operating in manual mode");

  pumpStatus = data["pump_status"].as<int>();
  lightStatus = data["light_status"].as<int>();
}

void updateOutputs() {
  digitalWrite(RELAY_PUMP_1, pumpStatus ? LOW : HIGH);
  digitalWrite(RELAY_PUMP_2, pumpStatus ? LOW : HIGH);
  digitalWrite(RELAY_LIGHT_1, lightStatus ? LOW : HIGH);
  digitalWrite(RELAY_LIGHT_2, lightStatus ? LOW : HIGH);
}

void initializeRelays() {
  pinMode(RELAY_PUMP_1, OUTPUT);
  pinMode(RELAY_PUMP_2, OUTPUT);
  pinMode(RELAY_LIGHT_1, OUTPUT);
  pinMode(RELAY_LIGHT_2, OUTPUT);

  digitalWrite(RELAY_PUMP_1, HIGH);
  digitalWrite(RELAY_PUMP_2, HIGH);
  digitalWrite(RELAY_LIGHT_1, HIGH);
  digitalWrite(RELAY_LIGHT_2, HIGH);
}
