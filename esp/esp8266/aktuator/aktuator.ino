#include <ESP8266WiFi.h>
// #include <WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

#define relayPin1 4   // D2
#define relayPin2 5   // D1
#define relayPin3 12  // lampu (D6)
#define relayPin4 14  // lampu (D5)

const char *ssid = "Podcast Area";
const char *password = "iriunwebcam";
const char *websocket_actuator = "ws://172.23.0.188:10000/actuator";
const char* moisttemp = "ws://172.23.0.188:10000/mois_temp";
bool actuator_connected = false;

String avgMoistureAPI = "http://172.23.0.188:15000/sensors/moistureAvg";

using namespace websockets;
WebsocketsClient client_actuator;
WebsocketsClient client_moisttemp;

int pumpstatus;
int lampstatus;
float temperature;
int otomationStatus = 1;

float temperatureAvg(float temperature1, float temperature2) {
  return (temperature1 + temperature2) / 2;
}

void onMessageCallback(WebsocketsMessage message) {
    String command = message.data();
    Serial.println("Received command: " + command);

    StaticJsonDocument<256> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, command);

    if (error) {
        Serial.println("JSON Parsing Failed!");
        return;
    }

    serializeJsonPretty(jsonDoc, Serial);
    Serial.println();

    // Cek dan hitung suhu jika tersedia
    if (jsonDoc.containsKey("temperature_atas") && jsonDoc.containsKey("temperature_bawah")) {
        float temperatureAtas = jsonDoc["temperature_atas"].as<float>();
        float temperatureBawah = jsonDoc["temperature_bawah"].as<float>();
        temperature = temperatureAvg(temperatureAtas, temperatureBawah);
        Serial.println("Temperature: " + String(temperature));
    }

    // Status otomasi
    if (jsonDoc.containsKey("otomationStatus")) {
        otomationStatus = jsonDoc["otomationStatus"].as<int>();
        Serial.println("Otomation Status: " + String(otomationStatus));
    }

    if (otomationStatus == 1) {
        Serial.println("Otomatis nyala");

        if (jsonDoc.containsKey("moistureAvg") && !isnan(jsonDoc["moistureAvg"].as<float>())) {
            if (jsonDoc["moistureAvg"].as<float>() < 65) {
                pumpstatus = 1;
                digitalWrite(relayPin1, LOW);
                digitalWrite(relayPin2, LOW);
            } else {
                pumpstatus = 0;
                digitalWrite(relayPin1, HIGH);
                digitalWrite(relayPin2, HIGH);
            }
        }

        if (temperature < 27) {
            lampstatus = 1;
            digitalWrite(relayPin3, LOW);
            digitalWrite(relayPin4, LOW);
        } else {
            lampstatus = 0;
            digitalWrite(relayPin3, HIGH);
            digitalWrite(relayPin4, HIGH);
        }
        return;
    }

    if (otomationStatus == 0) {
        Serial.println("Otomatis mati");

        if (jsonDoc.containsKey("pumpStatus")) {
            pumpstatus = jsonDoc["pumpStatus"].as<int>();
            digitalWrite(relayPin1, pumpstatus == 1 ? LOW : HIGH);
            digitalWrite(relayPin2, pumpstatus == 1 ? LOW : HIGH);
        }

        if (jsonDoc.containsKey("lightStatus")) {
            lampstatus = jsonDoc["lightStatus"].as<int>();
            digitalWrite(relayPin3, lampstatus == 1 ? LOW : HIGH);
            digitalWrite(relayPin4, lampstatus == 1 ? LOW : HIGH);
        }
    }
}

void sendData() {
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["pumpStatus"] = pumpstatus;
  jsonDoc["lightStatus"] = lampstatus;
  jsonDoc["otomationStatus"] = otomationStatus;

  String data;
  serializeJson(jsonDoc, data);

  client_actuator.send(data);
  Serial.println("Sent: " + data);
}

void checkWiFiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected, attempting to reconnect...");

    WiFi.disconnect();
    WiFi.begin(ssid, password); 
    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 10000; // Timeout 10 detik

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout)
    {
      Serial.println("Reconnecting...");
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Reconnected to WiFi!");
    }
    else
    {
      Serial.println("Failed to reconnect. Will retry later.");
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi, IP address:");
  Serial.println(WiFi.localIP());

  bool actuator_connected = client_actuator.connect(websocket_actuator);
  if (actuator_connected) {
    Serial.println("Connected to WebSocket Actuator Server");
    client_actuator.onMessage(onMessageCallback);
  } else {
    Serial.println("Failed to connect");
  }

  bool moisttemp_connected = client_moisttemp.connect(moisttemp);
  if (moisttemp_connected) {
    Serial.println("Connected to WebSocket moisttemp Server");
    client_moisttemp.onMessage(onMessageCallback);
  } else {
    Serial.println("Failed to connect");
  }

  pinMode(relayPin1, OUTPUT);  // pompa
  pinMode(relayPin2, OUTPUT);  // pompa
  pinMode(relayPin3, OUTPUT);  // lampu
  pinMode(relayPin4, OUTPUT);  // lampu

  digitalWrite(relayPin1, LOW);  // Ubah pin sesuai pompa
  digitalWrite(relayPin2, LOW);  // Ubah pin sesuai pompa
  digitalWrite(relayPin3, LOW);  // lampu
  digitalWrite(relayPin4, LOW);  // lampu
}

void loop() {
  client_actuator.poll();
  client_moisttemp.poll();

  if (!client_actuator.available()) {
    Serial.println("WebSocket actuator disconnected, attempting to reconnect...");
    if (client_actuator.connect(websocket_actuator)) {
      Serial.println("Reconnected to WebSocket actuator Server");
      client_actuator.onMessage(onMessageCallback);
    }
  }

  if (!client_moisttemp.available()) {
    Serial.println("WebSocket moisttemp disconnected, attempting to reconnect...");
    if (client_moisttemp.connect(moisttemp)) {
      Serial.println("Reconnected to WebSocket moisttemp Server");
      client_moisttemp.onMessage(onMessageCallback);
    }
  }

  checkWiFiConnection();

  static unsigned long lastSendTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= 10000) {
    sendData();
    lastSendTime = currentTime;
  }
}