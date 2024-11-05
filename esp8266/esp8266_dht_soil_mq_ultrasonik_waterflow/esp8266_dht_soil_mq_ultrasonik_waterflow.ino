#include <DHT.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

// Pin Definitions
// Ultrasonic Pin Section
#define TRIG_PIN 14 // Sensor Ultrasonik
#define ECHO_PIN 13 // Sensor Ultrasonik

// Sensor soil Pin Section
#define SENSOR_SOIL A0 // Sensor Soil

// DHT Pin Section
#define DHTPIN 13 // DHT

// MQ Pin Section
#define MQ_PIN 4 // MQ-135
#define LED_PIN 2       // Pin untuk LED (mengganti buzzer)

// Water Flow Pin Section
#define FLOW_SENSOR 5             // Pin untuk sensor aliran air (flow sensor)

// Motor DC Pin Section
#define PIN_ENA  14 // The ESP8266 pin connected to the EN1 pin L298N
#define PIN_IN1  12 // The ESP8266 pin connected to the IN1 pin L298N
#define PIN_IN2  13 // The ESP8266 pin connected to the IN2 pin L298N

#define PIN_RELAY_2  12 // The ESP8266 pin connected to the IN2 pin of relay module

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Konstanta
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

long duration;
float distanceCm;
float distanceInch;
float moisture_percentage;

// Water flow variables
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;

float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;

float flowRate;
float Litres;
float totalLitres = 0;  // Inisialisasi totalLitres dari 0

int statusMotor = 0;
int statusWaterPump = 0;

const char* ssid = "<SSID>";
const char* password = "<PASSWORD>";

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  // Inisialisasi Serial
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  
  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  
  // Inisialisasi pin untuk sensor ultrasonik, DHT, LED, dan sensor aliran air
  pinMode(TRIG_PIN, OUTPUT);  // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT);   // Sets the echoPin as an Input
  pinMode(MQ_PIN, INPUT);    // Pin untuk MQ Sensor
  pinMode(LED_PIN, OUTPUT);  // Pin untuk LED (sebagai pengganti buzzer)
  pinMode(FLOW_SENSOR, INPUT_PULLUP); // Pin untuk water flow sensor
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);

  dht.begin(); // Memulai sensor DHT

  pulseCount = 0;
  flowRate = 0.0;
  previousMillis = 0;

  // Atur interrupt untuk sensor aliran air
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);
}

void activateMotor() {
  statusMotor = 1;
  Serial.println("Jarak terdeteksi di bawah 5 cm, mengaktifkan motor DC");
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);

  // Gradually increase motor speed
  for (int speed = 0; speed <= 255; speed++) {
    analogWrite(PIN_ENA, speed);
    delay(10);
  }

  delay(10000);  // Motor berputar selama 10 detik

  // Gradually decrease motor speed to stop
  for (int speed = 255; speed >= 0; speed--) {
    analogWrite(PIN_ENA, speed);
    delay(10);
  }

  statusMotor = 0;
  Serial.println("Motor berhenti");
}

void activatePump() {
  if (statusMotor == 0) {  // Ensure pump only activates after motor has stopped
    Serial.println("Mengaktifkan relay untuk menghidupkan pompa");
    digitalWrite(PIN_RELAY_2, LOW);  // Activate relay (pump on)
    delay(8000);                     // Pump runs for 8 seconds
    digitalWrite(PIN_RELAY_2, HIGH); // Deactivate relay (pump off)
    Serial.println("Pompa berhenti");
  } else {
    Serial.println("Menunggu motor berhenti sebelum menghidupkan pompa");
  }
}

void checkDistanceAndOperate() {
  if (distanceCm < 5) {
    activateMotor();  // Start motor if distance is below threshold
    activatePump();   // Start pump after motor stops
  } else {
    Serial.println("Jarak lebih dari 5 cm, motor dan relay tidak aktif");
  }
}

void loop() {
  // Pembacaan suhu dan kelembapan dari sensor DHT
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  float MQValue = digitalRead(MQ_PIN);

  // Mengecek apakah data dari sensor berhasil dibaca
  if (isnan(suhu) || isnan(kelembapan) || isnan(MQValue)) {
    Serial.println("Gagal membaca data dari sensor");
  } else {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C, Kelembapan: ");
    Serial.print(kelembapan);
    Serial.println(" %");
    Serial.print("MQ Value: ");
    Serial.print(MQValue);
    Serial.println(" ppm");
  }

  // Pembacaan dari sensor ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration * SOUND_VELOCITY / 2;
  distanceInch = distanceCm * CM_TO_INCH;

  // Menampilkan jarak pada Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

// Clear the buffer.
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Jarak (cm): ");
  display.println(distanceCm);
  display.display();

  checkDistanceAndOperate();

  // Pembacaan sensor kelembapan tanah
  moisture_percentage = (100.00 - ((analogRead(SENSOR_SOIL) / 1023.00) * 100.00));
  Serial.print("Soil Moisture(in Percentage) = ");
  Serial.print(moisture_percentage);
  Serial.println("%");

  // Water flow sensor logic
  currentMillis = millis();
  
  // Mengecek setiap 1 detik
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;  // Simpan jumlah pulsa per detik
    pulseCount = 0;          // Reset hitungan pulsa untuk pengukuran berikutnya
    
    // Hitung laju aliran (liter per menit)
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

    // Menyalakan LED jika aliran air terdeteksi
    if (flowRate > 0) {
      digitalWrite(LED_PIN, HIGH);  // Menyalakan LED
      delay(500);
      digitalWrite(LED_PIN, LOW);   // Mematikan LED
      delay(500);
    }
    
  }
  // Delay sebelum pengulangan loop
  delay(1000);
}
