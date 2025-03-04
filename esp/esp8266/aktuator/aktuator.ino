#include <ESP8266WiFi.h>
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
const char* websocket_actuator = "ws://172.23.14.189:10000/actuator";
const char* webcommand = "ws://172.23.14.189:10000/webcommand";
bool actuator_connected = false;

String avgMoistureAPI = "http://172.23.14.189:15000/sensors/moistureAvg";

using namespace websockets;
WebsocketsClient client_actuator;
WebsocketsClient client_webcommand;

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

  if (jsonDoc["temperature_atas"] && jsonDoc["temperature_bawah"]) {
    float temperatureAtas = jsonDoc["temperature_atas"];
    float temperatureBawah = jsonDoc["temperature_bawah"];
    temperature = temperatureAvg(temperatureAtas, temperatureBawah);
    Serial.println("Temperature: " + String(temperature));
  }

  if (jsonDoc["otomationStatus"]) {
    otomationStatus = jsonDoc["otomationStatus"];
    Serial.println("Otomation Status: " + String(otomationStatus));
  }

  if (otomationStatus == 1) {
    Serial.println("Otomatis nyala");
    if (jsonDoc["moistureAvg"] < 65) {
      pumpstatus = 1;
      digitalWrite(relayPin1, LOW);  // Ubah pin sesuai pompa
      digitalWrite(relayPin2, LOW);  // Ubah pin sesuai pompa
    }
    if (jsonDoc["moistureAvg"] >= 65) {
      pumpstatus = 0;
      digitalWrite(relayPin1, HIGH);  // Ubah pin sesuai pompa
      digitalWrite(relayPin2, HIGH);  // Ubah pin sesuai pompa
    }
    if (temperature < 27) {
      lampstatus = 1;
      digitalWrite(relayPin3, LOW);  // Ubah pin sesuai lampu
      digitalWrite(relayPin4, LOW);  // Ubah pin sesuai lampu
    }
    if (temperature >= 27) {
      lampstatus = 0;
      digitalWrite(relayPin3, HIGH);  // Ubah pin sesuai lampu
      digitalWrite(relayPin4, HIGH);  // Ubah pin sesuai lampu
    }
  }
  if (otomationStatus == 0) {
    Serial.println("Otomatis mati");
    if (jsonDoc["pumpStatus"] == 1) {
      pumpstatus = 1;
      digitalWrite(relayPin1, LOW);  // Ubah pin sesuai pompa
      digitalWrite(relayPin2, LOW);  // Ubah pin sesuai pompa
    }
    if (jsonDoc["pumpStatus"] == 0) {
      pumpstatus = 0;
      digitalWrite(relayPin1, HIGH);  // Ubah pin sesuai pompa
      digitalWrite(relayPin2, HIGH);  // Ubah pin sesuai pompa
    }
    if (jsonDoc["lightStatus"] == 1) {
      lampstatus = 1;
      digitalWrite(relayPin3, LOW);  // Ubah pin sesuai lampu
      digitalWrite(relayPin4, LOW);  // Ubah pin sesuai lampu
    }
    if (jsonDoc["lightStatus"] == 0) {
      lampstatus = 0;
      digitalWrite(relayPin3, HIGH);  // Ubah pin sesuai lampu
      digitalWrite(relayPin4, HIGH);  // Ubah pin sesuai lampu
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

  bool webcommand_connected = client_webcommand.connect(webcommand);
  if (webcommand_connected) {
    Serial.println("Connected to WebSocket Webcommand Server");
    client_webcommand.onMessage(onMessageCallback);
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
  if (client_actuator.available()) {
    client_actuator.poll();
    sendData();
  } else {
    Serial.println("WebSocket disconnected, attempting to reconnect...");
    while (!client_actuator.connect(websocket_actuator)) {  // Loop hingga berhasil connect
      Serial.println("Reconnection failed, retrying...");
      delay(1000);
    }
    Serial.println("Reconnected to WebSocket actuator Server");
    client_actuator.onMessage(onMessageCallback);  // Pasang ulang callback

    while (!client_webcommand.connect(webcommand)) {  // Loop hingga berhasil connect
      Serial.println("Reconnection failed, retrying...");
      delay(1000);
    }
    Serial.println("Reconnected to WebSocket webcommand Server");
    client_webcommand.onMessage(onMessageCallback);  // Pasang ulang callback
  }

  checkWiFiConnection();

  delay(10000);
}