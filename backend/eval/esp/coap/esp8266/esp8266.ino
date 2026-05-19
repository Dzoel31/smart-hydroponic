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
const uint16_t localCoapPort = 5683;
const char *coapPathStatus = "coap/hydroponics/actuator";
const char *coapPathControl = "actuator/control";

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
void handleCoapControl(const char *payload);
void updateRelays();
void sendStatusUpdateCoap();
void checkConnections();
void callback_response(CoapPacket &packet, IPAddress ip, int port);
void callback_control(CoapPacket &packet, IPAddress ip, int port);

void setup() {
    Serial.begin(115200);

    // Initialize relay pins
    for (int pin : {RELAY_PUMP_1, RELAY_PUMP_2, RELAY_LIGHT_1, RELAY_LIGHT_2}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);  // OFF initially
    }

    connectToWifi();
    coap.response(callback_response);
    coap.server(callback_control, coapPathControl);
    coap.start(localCoapPort);
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

void callback_control(CoapPacket &packet, IPAddress ip, int port) {
    char payload[packet.payloadlen + 1];
    memcpy(payload, packet.payload, packet.payloadlen);
    payload[packet.payloadlen] = '\0';

    Serial.print("[CoAP] Control received: ");
    Serial.println(payload);

    handleCoapControl(payload);

    StaticJsonDocument<256> ackDoc;
    ackDoc["type"] = "actuator_ack";
    ackDoc["ack_type"] = "coap_control";
    ackDoc["pump_status"] = state.pump_status;
    ackDoc["light_status"] = state.light_status;
    ackDoc["automation_status"] = state.automation_status;
    ackDoc["device_id"] = DEVICE_ID;
    ackDoc["ack_time_ms"] = millis();

    StaticJsonDocument<256> requestDoc;
    if (deserializeJson(requestDoc, payload) == DeserializationError::Ok) {
        if (requestDoc.containsKey("command_id")) {
            ackDoc["command_id"] = requestDoc["command_id"];
        }
        if (requestDoc.containsKey("correlation_id")) {
            ackDoc["correlation_id"] = requestDoc["correlation_id"];
        }
    }

    String ackPayload;
    serializeJson(ackDoc, ackPayload);
    coap.sendResponse(ip, port, packet.messageid, (char *)ackPayload.c_str());
    Serial.println("[ACTUATOR_ACK] " + ackPayload);
}

void handleCoapControl(const char *payload) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.println("[CoAP] JSON parse error");
        return;
    }

    JsonVariant data;
    if (doc.containsKey("payload")) {
        data = doc["payload"];
    } else {
        data = doc.as<JsonVariant>();
    }

    if (data.containsKey("automation_status")) {
        state.automation_status = data["automation_status"].as<int>();
    }
    if (data.containsKey("moisture_avg")) {
        state.moisture_avg = data["moisture_avg"].as<float>();
    }
    if (data.containsKey("temperature_avg")) {
        state.temperature_avg = data["temperature_avg"].as<float>();
    }

    if (state.automation_status == 1) {
        if (!isnan(state.moisture_avg)) {
            state.pump_status = (state.moisture_avg < MOISTURE_THRESHOLD) ? 1 : 0;
        }
        if (!isnan(state.temperature_avg)) {
            state.light_status = (state.temperature_avg < TEMPERATURE_THRESHOLD) ? 1 : 0;
        }
    } else {
        if (data.containsKey("pump_status")) {
            state.pump_status = data["pump_status"].as<int>();
        }
        if (data.containsKey("light_status")) {
            state.light_status = data["light_status"].as<int>();
        }
    }

    updateRelays();
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
