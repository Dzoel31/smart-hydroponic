#include <WiFi.h>  // For ESP8266: #include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

const char* ssid = "duFIFA";
const char* password = "Fahri8013";
const char* websocket_server = "ws://192.168.1.16:10000";  // Change to your backend IP

using namespace websockets;
WebsocketsClient client;

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi Connected!");

    // Connect to WebSocket
    bool connected = client.connect(websocket_server);
    if (connected) {
        Serial.println("Connected to WebSocket Server");
    } else {
        Serial.println("Failed to connect");
    }
}

void loop() {
    if (client.available()) {
        // Simulate sensor data
        float temperature = random(20, 30);  // Fake temperature data
        float humidity = random(60, 90);     // Fake humidity data

        StaticJsonDocument<256> jsonDoc;
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;

        String data;
        serializeJson(jsonDoc, data);

        client.send(data);
        Serial.println("Sent: " + data);
        }

    client.poll();  // Check for incoming messages
    delay(5000);    // Send data every 5 seconds
}
