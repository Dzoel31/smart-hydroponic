#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
// #include <WiFi.h>

// Pin Definitions
#define RELAY_PUMP_1 4
#define RELAY_PUMP_2 5
#define RELAY_LIGHT_1 12
#define RELAY_LIGHT_2 14

// Configuration
const char *WIFI_SSID = "FIK-Hotspot";
const char *WIFI_PASSWORD = "T4nahairku";
const char *DEVICE_ID = "esp8266-actuator-device";
const unsigned long DATA_SEND_INTERVAL = 30000;
const unsigned long WIFI_RECONNECT_TIMEOUT = 10000;
const float MOISTURE_THRESHOLD = 60;
const float TEMPERATURE_THRESHOLD = 30.0;

// CoAP Configuration
IPAddress coapServerIp(172, 25, 21, 236);
const uint16_t coapServerPort = 8683;
const char *coapPathStatus = "coap/hydroponics/actuator";

int seq = 1;
unsigned long send_time = 0;
int last_seq_sent = 0;

// State variables
struct ActuatorState {
    int pump_status = 0;
    int light_status = 0;
    int automation_status = 0;
    float moisture_avg = 0;
    float temperature_avg = 0;
} state;

WiFiUDP udp;
Coap coap(udp, 512);

// Function prototypes
void connectToWifi();
void updateRelays();
void sendStatusUpdateCoap();
void checkConnections();
void callback_response(CoapPacket &packet, IPAddress ip, int port);

void setup() {
    Serial.begin(115200);

    // Initialize relay pins
    for (int pin : {RELAY_PUMP_1, RELAY_PUMP_2, RELAY_LIGHT_1, RELAY_LIGHT_2}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);  // OFF initially
    }

    connectToWifi();
    coap.response(callback_response);
    coap.start();
}

void loop() {
    checkConnections();
    coap.loop();

    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime >= DATA_SEND_INTERVAL) {
        sendStatusUpdateCoap();
        lastSendTime = millis();
    }
}

void connectToWifi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nFailed to connect");
    }
}

void updateRelays() {
    digitalWrite(RELAY_PUMP_1, state.pump_status ? LOW : HIGH);
    digitalWrite(RELAY_PUMP_2, state.pump_status ? LOW : HIGH);
    digitalWrite(RELAY_LIGHT_1, state.light_status ? LOW : HIGH);
    digitalWrite(RELAY_LIGHT_2, state.light_status ? LOW : HIGH);
}

void callback_response(CoapPacket &packet, IPAddress ip, int port) {
    Serial.println("[CoAP] Response received");

    if (last_seq_sent > 0) {
        unsigned long latency = millis() - send_time;
        Serial.printf("[METRIC] Seq: %d | Latency: %lu ms\n", last_seq_sent, latency);
    }

    if (packet.payloadlen > 0) {
        char payload[packet.payloadlen + 1];
        memcpy(payload, packet.payload, packet.payloadlen);
        payload[packet.payloadlen] = '\0';

        Serial.print("[CoAP] Payload: ");
        Serial.println(payload);
    }
}

void sendStatusUpdateCoap() {
    StaticJsonDocument<256> doc;
    doc["seq"] = seq;
    doc["pump_status"] = state.pump_status;
    doc["light_status"] = state.light_status;
    doc["automation_status"] = state.automation_status;
    doc["device_id"] = DEVICE_ID;

    String payload;
    serializeJson(doc, payload);

    last_seq_sent = seq;
    send_time = millis();

    coap.put(coapServerIp, coapServerPort, coapPathStatus, (char *)payload.c_str());

    seq++;
}

void checkConnections() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= WIFI_RECONNECT_TIMEOUT) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Reconnecting WiFi...");
            connectToWifi();
        }
        lastCheck = millis();
    }
}
