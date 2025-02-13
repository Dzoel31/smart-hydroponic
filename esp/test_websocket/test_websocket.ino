#include <WiFi.h> // Untuk ESP32, gunakan ESP8266WiFi.h jika ESP8266
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

const char *ssid = "duFIFA";
const char *password = "Fahri8013";
const char *websocket_server = "ws://192.168.1.9:10000"; // Ganti dengan IP backend
bool connected = false;

int pumpstatus = 0;
int lampstatus = 0;

using namespace websockets;
WebsocketsClient client;

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

    if (jsonDoc["pump_status"] == 1)
    {
        Serial.println("Turning ON the pump");
        // digitalWrite(5, HIGH); // Ubah pin sesuai pompa
    }
    else if (jsonDoc["pump_status"] == 0)
    {
        Serial.println("Turning OFF the pump");
        // digitalWrite(5, LOW); // Ubah pin sesuai pompa
    }
    else if (jsonDoc["lamp_status"] == 1)
    {
        Serial.println("Turning ON the lamp");
        // digitalWrite(4, HIGH); // Ubah pin sesuai lampu
    }
    else if (jsonDoc["lamp_status"] == 0)
    {
        Serial.println("Turning OFF the lamp");
        // digitalWrite(4, LOW); // Ubah pin sesuai lampu
    }
    else
    {
        Serial.println("Invalid command");
    }
}
void setup()
{
    Serial.begin(115200);

    // Koneksi WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi Connected!");

    // Koneksi WebSocket
    bool connected = client.connect(websocket_server);
    if (connected)
    {
        Serial.println("Connected to WebSocket Server");
        client.onMessage(onMessageCallback); // Pasang callback
    }
    else
    {
        Serial.println("Failed to connect");
    }
}

void loop()
{
    if (client.available())
    {
        client.poll();
    }
    else
    {
        Serial.println("WebSocket disconnected, attempting to reconnect...");
        while (!client.connect(websocket_server))
        { // Loop hingga berhasil connect
            Serial.println("Reconnection failed, retrying...");
            delay(3000);
        }
        Serial.println("Reconnected to WebSocket Server");
        client.onMessage(onMessageCallback); // Pasang ulang callback
    }

    // sendSensorData(); // Kirim data sensor tetap dilakukan
    delay(3000);
}
