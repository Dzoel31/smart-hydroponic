#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>  // Perbaikan: Gunakan library HTTP untuk ESP8266
#include <ArduinoJson.h>

#define moisturePin 13 // D7
#define relayPin1 4    // D2
#define relayPin2 5    // D1
#define relayPin3 12    // lampu (D6)
#define relayPin4 14    // lampu (D5)
// #define enaPin 2       // D4
// #define in1Pin 14      // D5
// #define in2Pin 12      // D6 (sebelumnya 13, diperbaiki karena konflik dengan moisturePin)

int moisture, moistureAnalog;
const char* ssid = "FIK-Hotspot";
const char* password = "T4nahairku";
String serverName = "http://172.23.13.248:8000/api/get_latest_data";  // URL server untuk menerima status pompa

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi, IP address:");
  Serial.println(WiFi.localIP());

  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  pinMode(relayPin4, OUTPUT);
}

void loop(void) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;  // Diperlukan untuk HTTPClient di ESP8266
    HTTPClient http;
    
    http.begin(client, serverName);  // Perbaikan: Tambahkan `client`
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from server: " + response);

      // Parsing JSON dari respons server
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, response);

      if (!error) {
        // Cetak JSON dalam format rapi untuk debugging
        serializeJsonPretty(jsonDoc, Serial);
        Serial.println();

        // Pastikan status pompa ada dalam JSON
        if (jsonDoc.containsKey("data") && jsonDoc["data"].containsKey("pump_status")) {
          int pumpStatus = jsonDoc["data"]["pump_status"];

          Serial.print("Status Pompa dari Server: ");
          Serial.println(pumpStatus);

          // Kontrol pompa berdasarkan status dari server
          if (pumpStatus == 1) {
            digitalWrite(relayPin2, LOW);
            digitalWrite(relayPin1, LOW);
            digitalWrite(relayPin3, LOW);
            digitalWrite(relayPin4, LOW);
            Serial.println("Pompa dinyalakan.");
          } else {
            digitalWrite(relayPin2, HIGH);
            digitalWrite(relayPin1, HIGH);
            digitalWrite(relayPin3, HIGH);
            digitalWrite(relayPin4, HIGH);
            Serial.println("Pompa dimatikan.");
          }
        } else {
          Serial.println("⚠️  Kesalahan: Key 'pump_status' tidak ditemukan dalam JSON!");
        }
      } else {
        Serial.println("⚠️  Error parsing JSON: " + String(error.c_str()));
      }
    } else {
      Serial.print("❌ Error retrieving data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("⚠️  WiFi disconnected, retrying...");
  }

  delay(7000);
}
