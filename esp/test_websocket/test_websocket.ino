#include <WiFi.h>  // Untuk ESP32, gunakan ESP8266WiFi.h jika ESP8266
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

const char* ssid = "FIK-Dekanat";
const char* password = "F4silkom";
const char* websocket_server = "ws://172.23.13.115:10000";  // Ganti dengan IP backend

using namespace websockets;
WebsocketsClient client;

void onMessageCallback(WebsocketsMessage message) {
    String msg = message.data();
    Serial.println("Received: " + msg);

    // Parse JSON yang diterima
    StaticJsonDocument<256> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, msg);

    if (error) {
        Serial.println("JSON Parsing Failed!");
        return;
    }

    // Ambil data dari JSON
    if (jsonDoc.containsKey("command")) {
        String command = jsonDoc["command"].as<String>();
        Serial.println("Command received: " + command);

        if (command == "get_status") {
            sendSensorData();  // Kirim ulang data sensor jika diminta
        }
    }
}

void sendSensorData() {
    // Simulasi data sensor
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

void setup() {
    Serial.begin(115200);

    // Koneksi WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi Connected!");

    // Koneksi WebSocket
    bool connected = client.connect(websocket_server);
    if (connected) {
        Serial.println("Connected to WebSocket Server");
        client.onMessage(onMessageCallback);  // Pasang callback
    } else {
        Serial.println("Failed to connect");
    }
}

void loop() {
    client.poll();  // Cek pesan masuk dari server

    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime > 5000) {  // Kirim data setiap 5 detik
        sendSensorData();
        lastSendTime = millis();
    }
}
