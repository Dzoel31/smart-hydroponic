#define BLYNK_TEMPLATE_ID "TMPL68FSq4d8e"
#define BLYNK_TEMPLATE_NAME "Smart Hidroponik"
#define BLYNK_AUTH_TOKEN "5cmX2SdrFnscq7WZvCXxWuEfxTbkEoHK"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>  // Perbaikan: Gunakan library HTTP untuk ESP8266
#include <ArduinoJson.h>

#define moisturePin 13  // D7
#define relayPin1 4     // D2
#define relayPin2 5     // D1
#define relayPin3 12    // lampu (D6)
#define relayPin4 14    // lampu (D5)
// #define enaPin 2       // D4
// #define in1Pin 14      // D5
// #define in2Pin 12      // D6 (sebelumnya 13, diperbaiki karena konflik dengan moisturePin)

// int moisture, moistureAnalog;
const char* ssid = "FIK-Hotspot";
const char* password = "T4nahairku";
String lampAPI = "https://blynk.cloud/external/api/get?token=5cmX2SdrFnscq7WZvCXxWuEfxTbkEoHK&V5";
String avgMoistureAPI = "https://blynk.cloud/external/api/get?token=5cmX2SdrFnscq7WZvCXxWuEfxTbkEoHK&V0";  // URL server untuk menerima status pompa

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

  pinMode(relayPin1, OUTPUT); // pompa
  pinMode(relayPin2, OUTPUT); // pompa
  pinMode(relayPin3, OUTPUT); // lampu
  pinMode(relayPin4, OUTPUT); // lampu
}

void loop(void) {
  if (WiFi.status() == WL_CONNECTED) {

    Blynk.run();
    getLampStatus();
    getMoisture();
  }

  delay(5000);
}

void getLampStatus() {
  HTTPClient http;
  http.begin(lampAPI);

  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCOde == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Response from Lamp Endpoint: " + payload);
    }
  }
  http.end();
}

void getMoisture() {
  HTTPClient http;
  http.begin(avgMoistureAPI);

  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCOde == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Response from Moisture Endpoint: " + payload);
    }
  }
  http.end();
}
