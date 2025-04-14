#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>

// Pin definitions
// Moisture sensors
#define MOISTURE_PIN1 32 // BIRU 1
#define MOISTURE_PIN2 33 // HIJAU 2
#define MOISTURE_PIN3 34 // input only // OREN 3
#define MOISTURE_PIN4 35 // input only // BIRU 4
#define MOISTURE_PIN5 36 // HIJAU bawah 5
#define MOISTURE_PIN6 39 // input only (SM) // COKELAT 6

// Water flow and ultrasonic sensor
#define WATERFLOW_PIN 16 // OREN
#define TRIGGER_PIN 18   // BIRU
#define ECHO_PIN 19

// Constants
#define SOUND_SPEED 0.034
#define JARAK_SENSOR_KE_DASAR 40
#define FLOW_CALIBRATION_FACTOR 4.5
#define DAY_IN_MS 86400000

// Network configuration
const char *WIFI_SSID = "Podcast Area";
const char *WIFI_PASSWORD = "iriunwebcam";
const char *WS_SERVER_URL = "ws://103.147.92.179:15000/device";
const char *DEVICE_ID = "esp32-plant-device";

// Time intervals
const unsigned long FLOW_INTERVAL = 1000;    // 1 second
const unsigned long ULTRASONIC_INTERVAL = 500; // 0.5 second
const unsigned long SEND_INTERVAL = 5000;    // 5 seconds
const unsigned long WIFI_TIMEOUT = 10000;    // 10 seconds

// Global variables
// Moisture sensor readings
int moisture[6] = {0, 0, 0, 0, 0, 0};
int moistureAnalog[6] = {0, 0, 0, 0, 0, 0};

// Water measurements
float flowRate = 0.0;
float totalLitres = 0.0;
volatile int pulseCount = 0;
float waterLevel = 0.0;

// Timing variables
unsigned long lastFlowCheck = 0;
unsigned long lastUltrasonicCheck = 0;
unsigned long lastSendTime = 0;

// WebSocket client
using namespace websockets;
WebsocketsClient client;

// Function prototypes
void connectToWifi();
void connectToWebSocket();
void readMoistureSensors();
void readWaterLevel();
void readWaterFlow();
void sendSensorData();
void resetDailyCounters();
void checkConnections();

// ISR for pulse counting
void IRAM_ATTR pulseCounter()
{
	pulseCount++;
}

void setup()
{
	Serial.begin(115200);
	
	// Setup pins
	pinMode(WATERFLOW_PIN, INPUT_PULLUP);
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);
	
	// Setup interrupt
	attachInterrupt(digitalPinToInterrupt(WATERFLOW_PIN), pulseCounter, FALLING);
	
	// Connect to WiFi and WebSocket
	connectToWifi();
	connectToWebSocket();
}

void loop()
{
	unsigned long currentMillis = millis();
	
	// Check connections
	checkConnections();
	
	if (client.available()) {
		// Read sensors at appropriate intervals
		if (currentMillis - lastFlowCheck >= FLOW_INTERVAL) {
			readWaterFlow();
			lastFlowCheck = currentMillis;
		}
		
		if (currentMillis - lastUltrasonicCheck >= ULTRASONIC_INTERVAL) {
			readWaterLevel();
			readMoistureSensors();
			lastUltrasonicCheck = currentMillis;
		}
		
		if (currentMillis - lastSendTime >= SEND_INTERVAL) {
			sendSensorData();
			lastSendTime = currentMillis;
		}
		
		// Reset counters daily
		if (currentMillis >= DAY_IN_MS) {
			resetDailyCounters();
		}
	} else {
		// Reconnect if client is unavailable
		Serial.println("Client not available");
		connectToWebSocket();
	}
}

void connectToWifi()
{
	Serial.print("Connecting to ");
	Serial.println(WIFI_SSID);
	
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.println("Connecting to WiFi...");
	}
	
	Serial.println("\nConnected to WiFi, IP address:");
	Serial.println(WiFi.localIP());
}

void connectToWebSocket()
{
	Serial.println("Connecting to WebSocket server...");
	
	while (!client.connect(WS_SERVER_URL)) {
		Serial.println("WebSocket connection failed, retrying...");
		delay(1000);
	}
	
	Serial.println("Connected to WebSocket Server");
	
	// Register device
	StaticJsonDocument<256> registerJson;
	registerJson["device_id"] = DEVICE_ID;
	registerJson["type"] = "register";
	
	String registerString;
	serializeJson(registerJson, registerString);
	
	Serial.println("Registering device: " + registerString);
	client.send(registerString);
}

void readMoistureSensors()
{
	const int pins[6] = {MOISTURE_PIN1, MOISTURE_PIN2, MOISTURE_PIN3, 
												MOISTURE_PIN4, MOISTURE_PIN5, MOISTURE_PIN6};
	
	for (int i = 0; i < 6; i++) {
		moistureAnalog[i] = analogRead(pins[i]);
		moisture[i] = (100 - ((moistureAnalog[i] / 4095.0) * 100));
	}
}

void readWaterLevel()
{
	digitalWrite(TRIGGER_PIN, LOW);
	delayMicroseconds(2);
	
	digitalWrite(TRIGGER_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIGGER_PIN, LOW);
	
	long duration = pulseIn(ECHO_PIN, HIGH);
	waterLevel = JARAK_SENSOR_KE_DASAR - (duration * SOUND_SPEED / 2);
}

void readWaterFlow()
{
	detachInterrupt(WATERFLOW_PIN);
	
	flowRate = (pulseCount / FLOW_CALIBRATION_FACTOR);
	float litersPerSecond = flowRate / 60;
	totalLitres += litersPerSecond;
	pulseCount = 0;
	
	attachInterrupt(digitalPinToInterrupt(WATERFLOW_PIN), pulseCounter, FALLING);
}

void sendSensorData()
{
	StaticJsonDocument<256> jsonDoc;
	jsonDoc["device_id"] = DEVICE_ID;
	jsonDoc["type"] = "update_data";
	
	JsonObject dataObj = jsonDoc.createNestedObject("data");
	dataObj["moisture1"] = moisture[0];
	dataObj["moisture2"] = moisture[1];
	dataObj["moisture3"] = moisture[2];
	dataObj["moisture4"] = moisture[3];
	dataObj["moisture5"] = moisture[4];
	dataObj["moisture6"] = moisture[5];
	dataObj["flowRate"] = flowRate;
	dataObj["totalLitres"] = totalLitres;
	dataObj["distanceCm"] = waterLevel;
	
	String jsonString;
	serializeJson(jsonDoc, jsonString);
	
	client.send(jsonString);
	Serial.println("Sent: " + jsonString);
}

void resetDailyCounters()
{
	totalLitres = 0;
	lastFlowCheck = 0;
	lastUltrasonicCheck = 0;
	lastSendTime = 0;
}

void checkConnections()
{
	// Check WiFi connection
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("WiFi disconnected, attempting to reconnect...");
		
		WiFi.disconnect();
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		unsigned long startAttemptTime = millis();
		
		while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
			Serial.println("Reconnecting...");
			delay(500);
		}
		
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("Reconnected to WiFi!");
		} else {
			Serial.println("Failed to reconnect. Will retry later.");
		}
	}
}
