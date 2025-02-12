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
const char *websocket_server = "ws://172.23.13.115:10000";
const char *type_sensor = "pump_lamp_ESP8266";

String lampAPI = "https://blynk.cloud/external/api/get?token=5cmX2SdrFnscq7WZvCXxWuEfxTbkEoHK&V5";
String avgMoistureAPI = "http://172.23.13.115:15000/sensors/avg_moisture";
String pumpAPI = "https://blynk.cloud/external/api/get?token=5cmX2SdrFnscq7WZvCXxWuEfxTbkEoHK&V2";

WiFiClientSecure client;
using namespace websockets;
WebsocketsClient wsclient;

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

  pinMode(relayPin1, OUTPUT); // pompa
  pinMode(relayPin2, OUTPUT); // pompa
  pinMode(relayPin3, OUTPUT); // lampu
  pinMode(relayPin4, OUTPUT); // lampu

  client.setInsecure();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
    getLampStatus();
    getMoisture();
    activatePump();
  }
  delay(5000);
}

void getLampStatus() {
  HTTPClient http;

  http.begin(client, lampAPI); 
  int httpCode = http.GET();
  if (httpCode > 0) {
      String payload = http.getString();
      if (payload == "1") {
        digitalWrite(relayPin3, LOW);
        digitalWrite(relayPin4, LOW);
      } else {
        digitalWrite(relayPin3, HIGH);
        digitalWrite(relayPin4, HIGH);
      }
  }
  http.end();
}

void getMoisture() {
  HTTPClient http;

  http.begin(client, avgMoistureAPI);

  int httpCode = http.GET();
  if (httpCode > 0) {
      String payload = http.getString();
      if (payload < "50") {
        digitalWrite(relayPin1, LOW);
        digitalWrite(relayPin2, LOW);
        Blynk.virtualWrite(V2, 1);
      } else {
        digitalWrite(relayPin1, HIGH);
        digitalWrite(relayPin2, HIGH);
      }

  }
  http.end();
}

void activatePump() {
  HTTPClient http;

  http.begin(client, pumpAPI);

  int httpCode = http.GET();
  if (httpCode > 0) {
      String payload = http.getString();
      if (payload == "1") {
        digitalWrite(relayPin1, LOW);
        digitalWrite(relayPin2, LOW);
      } else {
        digitalWrite(relayPin1, HIGH);
        digitalWrite(relayPin2, HIGH);
      }
  }
  http.end();
}