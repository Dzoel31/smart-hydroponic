#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "DHT.h"

// Pin and sensor definitions
#define TDS_SENSOR_PIN 34
#define VREF 3.3
#define SCOUNT 30
#define DHT11_PIN 14
#define DHT22_PIN 26
#define DHT11_TYPE DHT11
#define DHT22_TYPE DHT22

// Network configuration
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";
const char *server_url = "";
const char *device_id = "esp32-environment-device";

// Timing constants
const unsigned long SEND_INTERVAL = 5000; // 5 seconds
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
float phValue = 0; // Placeholder for pH sensor value
float temperature_atas = 0;
float humidity_atas = 0;
float temperature_bawah = 0;
float humidity_bawah = 0;

// Objects
DHT dht11(DHT11_PIN, DHT11_TYPE);
DHT dht22(DHT22_PIN, DHT22_TYPE);

// Function prototypes
int getMedianNum(int bArray[], int iFilterLen);
void checkWiFiConnection();
void readDHTSensors();
void readTDSSensor();
// void readPHSensor(); // Untuk membaca sensor pH jika ada
void sendSensorData();

void setup() {
  Serial.begin(115200);
  pinMode(TDS_SENSOR_PIN, INPUT);

  // Initialize sensors
  dht11.begin();
  dht22.begin();

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println("\nConnected to WiFi");
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
  int median = (iFilterLen & 1) ? bTab[(iFilterLen - 1) / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  Serial.print("Median value: "); Serial.println(median);
  return median;
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

void readDHTSensors() {
  temperature_atas = dht11.readTemperature();
  humidity_atas = dht11.readHumidity();
  temperature_bawah = dht22.readTemperature();
  humidity_bawah = dht22.readHumidity();

  if (isnan(temperature_atas))
    temperature_atas = 0;
  if (isnan(humidity_atas))
    humidity_atas = 0;
  if (isnan(temperature_bawah))
    temperature_bawah = 0;
  if (isnan(humidity_bawah))
    humidity_bawah = 0;
}

void readPHSensor() {
  // Kode membaca sensor pH disini
  phValue = 0;
}

void readTDSSensor() {
  for (int i = 0; i < SCOUNT; i++) {
    analogBufferTemp[i] = analogBuffer[i];
  }

  float medianRaw = getMedianNum(analogBufferTemp, SCOUNT);
  float averageVoltage = medianRaw * VREF / 4095.0;
  float compensationCoefficient = 1.0 + 0.02 * (temperature_bawah - 25.0); // Using temperature_bawah for compensation
  float compensationVoltage = averageVoltage / compensationCoefficient;
  tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage);

  Serial.print("TDS medianRaw: ");
  Serial.println(medianRaw);
  Serial.print("TDS averageVoltage: ");
  Serial.println(averageVoltage, 4);
  Serial.print("TDS compensationCoefficient: ");
  Serial.println(compensationCoefficient, 4);
  Serial.print("TDS compensationVoltage: ");
  Serial.println(compensationVoltage, 4);
  Serial.print("TDS value: ");
  Serial.println(tdsValue, 2);
}

void sendSensorData() {
  HTTPClient http;
  http.begin(server_url);

  StaticJsonDocument<256> jsonDoc;
  jsonDoc["device_id"] = device_id;
  jsonDoc["type"] = "update_data";

  JsonObject data = jsonDoc.createNestedObject("data");
  data["temperatureAtas"] = temperature_atas;
  data["humidityAtas"] = humidity_atas;
  data["temperatureBawah"] = temperature_bawah;
  data["humidityBawah"] = humidity_bawah;
  data["ph"] = phValue;
  data["tds"] = tdsValue;

  String dataStr;
  serializeJson(jsonDoc, dataStr);

  Serial.println("Sending data: " + dataStr);

  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(dataStr);

  if (httpResponseCode > 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Failed to send data");
  }

  http.end();
}

void loop() {
  unsigned long currentMillis = millis();

  // Sample TDS sensor at regular intervals
  if (currentMillis - lastAnalogSampleTime > TDS_SAMPLE_INTERVAL) {
    lastAnalogSampleTime = currentMillis;
    analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;

    // Only update TDS value after new sample
    readTDSSensor();
  }

  // Send data and print debug at regular intervals
  if (currentMillis - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = currentMillis;

    // Read DHT sensors
    readDHTSensors();
    // readPHSensor(); // Uncomment if pH sensor is available

    // Debug print every 5 seconds
    Serial.print("DHT11: Temp = ");
    Serial.print(temperature_atas);
    Serial.print(" C, Humidity = ");
    Serial.println(humidity_atas);
    Serial.print("DHT22: Temp = ");
    Serial.print(temperature_bawah);
    Serial.print(" C, Humidity = ");
    Serial.println(humidity_bawah);
    Serial.print("TDS value: ");
    Serial.println(tdsValue, 2);

    sendSensorData();
  }

  // Maintain connections
  checkWiFiConnection();
}
