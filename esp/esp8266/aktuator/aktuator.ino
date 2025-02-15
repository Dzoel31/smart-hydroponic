#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

#define relayPin1 4     // D2
#define relayPin2 5     // D1
#define relayPin3 12    // lampu (D6)
#define relayPin4 14    // lampu (D5)

const char* ssid = "FIK-Dekanat";
const char* password = "F4silkom";
const char *websocket_server = "ws://172.23.0.188:10000";
const char *type_sensor = "pump_light_ESP8266";
bool connected = false;

String avgMoistureAPI = "http://172.23.0.188:15000/sensors/moistureAvg";

using namespace websockets;
WebsocketsClient client;

int pumpstatus;
int lampstatus;
float temperature = 0;

float temperatureAvg(float temperature1, float temperature2) {
  return (temperature1 + temperature2) / 2;
}

void onMessageCallback(WebsocketsMessage message)
{
  String command = message.data();
  Serial.println("Received command: " + command);
  // Parse JSON dari server
  StaticJsonDocument<256> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, command);

  if (error)
  {
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

  if (jsonDoc["pumpStatus"] == 1 || jsonDoc["moistureAvg"] < 55)
  {
    pumpstatus = 1;
    digitalWrite(relayPin1, LOW); // Ubah pin sesuai pompa
    digitalWrite(relayPin2, LOW); // Ubah pin sesuai pompa
    delay(15000);
  }
  if (jsonDoc["pumpStatus"] == 0 || jsonDoc["moistureAvg"] >= 55)
  {
    pumpstatus = 0;
    digitalWrite(relayPin1, HIGH); // Ubah pin sesuai pompa
    digitalWrite(relayPin2, HIGH); // Ubah pin sesuai pompa
  }
  if (jsonDoc["lightStatus"] == 1 || temperature < 27 && lampstatus == 0)
  {
    lampstatus = 1;
    digitalWrite(relayPin3, LOW); // Ubah pin sesuai lampu
    digitalWrite(relayPin4, LOW); // Ubah pin sesuai lampu
  }
  if (jsonDoc["lightStatus"] == 0 || temperature >= 27 && lampstatus == 1)
  {
    lampstatus = 0;
    digitalWrite(relayPin3, HIGH); // Ubah pin sesuai lampu
    digitalWrite(relayPin4, HIGH); // Ubah pin sesuai lampu
  }
}

void sendData() {
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["type"] = type_sensor;
  jsonDoc["pumpStatus"] = pumpstatus;
  jsonDoc["lightStatus"] = lampstatus;

  String data;
  serializeJson(jsonDoc, data);

  client.send(data);
  Serial.println("Sent: " + data);
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

  bool connected = client.connect(websocket_server);
  if (connected)
  {
    Serial.println("Connected to WebSocket Server");
    client.onMessage(onMessageCallback);
  }
  else
  {
    Serial.println("Failed to connect");
  }

  pinMode(relayPin1, OUTPUT); // pompa
  pinMode(relayPin2, OUTPUT); // pompa
  pinMode(relayPin3, OUTPUT); // lampu
  pinMode(relayPin4, OUTPUT); // lampu

  digitalWrite(relayPin1, LOW); // Ubah pin sesuai pompa
  digitalWrite(relayPin2, LOW); // Ubah pin sesuai pompa
  digitalWrite(relayPin3, LOW); // lampu
  digitalWrite(relayPin4, LOW); // lampu
}

void loop() {
  if (client.available())
  {
    client.poll();
    sendData();
  }
  else
  {
    Serial.println("WebSocket disconnected, attempting to reconnect...");
    while (!client.connect(websocket_server))
    { // Loop hingga berhasil connect
      Serial.println("Reconnection failed, retrying...");
      delay(1000);
    }
    Serial.println("Reconnected to WebSocket Server");
    client.onMessage(onMessageCallback); // Pasang ulang callback
  }

  // sendData(); // Kirim data sensor tetap dilakukan
  delay(3000);
}