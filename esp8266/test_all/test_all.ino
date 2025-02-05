#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#define moisturePin 34
#define waterflowPin 32

int moisture, moistureAnalog;

float flowRate;
float Litres;
float totalLitres = 0; 
int pump_status = 0;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;

const char* ssid = "FIK-Hotspot";
const char* password = "T4nahairku";
String api_url = "http://172.23.13.248:8000/api/store_data";

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("\nConnected to WiFi, IP address:");
  Serial.println(WiFi.localIP());


  pinMode(waterflowPin, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(waterflowPin), pulseCounter, FALLING);
}

void loop(void) {

  currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    
    // Menghitung volume air dalam liter
    Litres = (flowRate / 60);  
    totalLitres += Litres;  // Tambahkan volume air ke total volume

    // Cetak hasil ke Serial Monitor
    Serial.print("Flow rate: ");
    Serial.print(flowRate);  // Laju aliran dalam liter per menit (L/min)
    Serial.print(" L/min");
    Serial.print("\t");      // Cetak tabulasi
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalLitres);  // Total volume air dalam liter
    Serial.println(" L");
  }
  
  moistureAnalog = analogRead(moisturePin);
  moisture = (100 - ((moistureAnalog / 4095.0) * 100));
  Serial.print("Moisture = ");
  Serial.print(moisture);
  Serial.println("%");

  Serial.print("Connecting to: ");
  Serial.println(api_url);

  if (moisture < 60) {
    pump_status = 1;
  } else {
    pump_status = 0;
  }

  currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    
    // Menghitung volume air dalam liter
    Litres = (flowRate / 60);  
    totalLitres += Litres;  // Tambahkan volume air ke total volume

    // Cetak hasil ke Serial Monitor
    Serial.print("Flow rate: ");
    Serial.print(flowRate);  // Laju aliran dalam liter per menit (L/min)
    Serial.print(" L/min");
    Serial.print("\t");      // Cetak tabulasi
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalLitres);  // Total volume air dalam liter
    Serial.println(" L");
  }

  HTTPClient http;

  String url = String(api_url);

  if (http.begin(url)) {
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> doc;

    doc["moisture"] = moisture;
    doc["pump_status"] = pump_status;
    doc["flow_rate"] = flowRate;
    doc["total_litres"] = totalLitres;

    String payload;
    serializeJson(doc, payload);

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println(httpResponseCode);
          Serial.println(response);
        } else {
          Serial.print("Error on sending POST: ");
          Serial.println(httpResponseCode);
        }

    http.end();
  } else {
    Serial.println("Error on HTTP request");
  }
  delay(5000);
}