#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <DHT.h>

// --- KONFIGURASI ---
// Pin dan Sensor
#define TDS_SENSOR_PIN 34
#define VREF 3.3
#define SCOUNT 30
#define DHT11_PIN_ATAS 33
#define DHT11_PIN_BAWAH 32
#define DHT11_TYPE DHT11

// Konfigurasi Jaringan (Silakan isi dengan data Anda)
const char *WIFI_SSID = "Podcast Area";
const char *WIFI_PASSWORD = "iriunwebcam";
const char *server_url = "ws://103.147.92.179";
const char *device_id = "esp32-environment-device";

// Konfigurasi Waktu
const unsigned long SEND_INTERVAL = 5000;      // 5 detik
const unsigned long WIFI_TIMEOUT = 15000;      // 15 detik
const unsigned long TDS_SAMPLE_INTERVAL = 40;  // 40 milidetik

// --- VARIABEL GLOBAL ---
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
unsigned long lastSendTime = 0;
unsigned long lastAnalogSampleTime = 0;

// Variabel penampung nilai sensor
float tdsValue = 0;
float phValue = 0; // Placeholder
float temperature_atas = 0;
float humidity_atas = 0;
float temperature_bawah = 0;
float humidity_bawah = 0;

// Objek library
DHT dht11Atas(DHT11_PIN_ATAS, DHT11_TYPE);
DHT dht11Bawah(DHT11_PIN_BAWAH, DHT11_TYPE);
using namespace websockets;
WebsocketsClient client;

// =================================================================
//                        FUNGSI SETUP
// =================================================================
void setup()
{
  Serial.begin(115200);
  pinMode(TDS_SENSOR_PIN, INPUT);

  // Inisialisasi sensor DHT
  dht11Atas.begin();
  dht11Bawah.begin();

  // --- LOGIKA KONEKSI WIFI ---
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Menghubungkan ke WiFi ..");
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT)
  {
    delay(500);
    Serial.print('.');
  }

  // Cek hasil koneksi WiFi
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nTerhubung ke WiFi!");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());

    // --- LOGIKA KONEKSI WEBSOCKET ---
    Serial.println("Menghubungkan ke WebSocket server...");
    if (client.connect(server_url))
    {
      Serial.println("Terhubung ke WebSocket Server!");

      // --- LOGIKA REGISTRASI PERANGKAT ---
      StaticJsonDocument<256> registerJson;
      registerJson["deviceId"] = device_id;
      registerJson["type"] = "join";
      registerJson["room"] = "environment";

      String registerString;
      serializeJson(registerJson, registerString);

      Serial.println("Registrasi perangkat: " + registerString);
      client.send(registerString);
    }
    else
    {
      Serial.println("Gagal terhubung ke WebSocket Server.");
    }
  }
  else
  {
    Serial.println("\nGagal terhubung ke WiFi. Silakan restart perangkat.");
    // Program akan berhenti di sini jika WiFi gagal, loop() tidak akan berjalan efektif.
  }
}


// =================================================================
//                         FUNGSI LOOP
// =================================================================
void loop()
{
  unsigned long currentMillis = millis();

  // --- LOGIKA PEMELIHARAAN KONEKSI WIFI ---
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Koneksi WiFi terputus, mencoba menyambungkan kembali...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT)
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nBerhasil terhubung kembali ke WiFi!");
    }
    else
    {
      Serial.println("\nGagal menyambung kembali.");
    }
  }

  // --- LOGIKA PEMBACAAN SENSOR TDS (NON-BLOCKING) ---
  if (currentMillis - lastAnalogSampleTime > TDS_SAMPLE_INTERVAL)
  {
    lastAnalogSampleTime = currentMillis;
    analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
  }

  // --- LOGIKA PEMBACAAN DAN PENGIRIMAN DATA BERKALA ---
  if (currentMillis - lastSendTime >= SEND_INTERVAL)
  {
    lastSendTime = currentMillis;

    // --- 1. LOGIKA PEMBACAAN SENSOR DHT ---
    temperature_atas = dht11Atas.readTemperature();
    humidity_atas = dht11Atas.readHumidity();
    temperature_bawah = dht11Bawah.readTemperature();
    humidity_bawah = dht11Bawah.readHumidity();

    if (isnan(temperature_atas)) temperature_atas = 0;
    if (isnan(humidity_atas)) humidity_atas = 0;
    if (isnan(temperature_bawah)) temperature_bawah = 0;
    if (isnan(humidity_bawah)) humidity_bawah = 0;
    
    // --- 2. LOGIKA KALKULASI NILAI TDS ---
    for (int i = 0; i < SCOUNT; i++)
    {
      analogBufferTemp[i] = analogBuffer[i];
    }
    // Proses sorting untuk mencari nilai median (pengganti fungsi getMedianNum)
    for (int j = 0; j < SCOUNT - 1; j++)
    {
      for (int i = 0; i < SCOUNT - j - 1; i++)
      {
        if (analogBufferTemp[i] > analogBufferTemp[i + 1])
        {
          int bTemp = analogBufferTemp[i];
          analogBufferTemp[i] = analogBufferTemp[i + 1];
          analogBufferTemp[i + 1] = bTemp;
        }
      }
    }
    float medianRaw = (SCOUNT & 1) ? analogBufferTemp[(SCOUNT - 1) / 2] : (analogBufferTemp[SCOUNT / 2] + analogBufferTemp[SCOUNT / 2 - 1]) / 2;
    
    // Konversi ke nilai TDS
    float averageVoltage = medianRaw * VREF / 4095.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature_bawah - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage);


    // --- 3. LOGIKA PENGIRIMAN DATA VIA WEBSOCKET ---
    StaticJsonDocument<512> jsonDoc;
    jsonDoc["deviceId"] = device_id;
    jsonDoc["type"] = "update_data";
    jsonDoc["room"] = "environment";
    jsonDoc["broadcast"] = "command";

    JsonObject data = jsonDoc.createNestedObject("data");
    data["temperatureAtas"] = temperature_atas;
    data["humidityAtas"] = humidity_atas;
    data["temperatureBawah"] = temperature_bawah;
    data["humidityBawah"] = humidity_bawah;
    data["tds"] = tdsValue; // Menambahkan data TDS
    data["ph"] = phValue;   // Menambahkan data pH (placeholder)

    String dataStr;
    serializeJson(jsonDoc, dataStr);

    if (client.available())
    {
      client.send(dataStr);
      Serial.println("Terkirim: " + dataStr);
    }
    else
    {
      Serial.println("Koneksi WebSocket tidak tersedia, mencoba menyambungkan ulang...");
      if (client.connect(server_url))
      {
        Serial.println("Berhasil terhubung kembali ke WebSocket!");
        // Kirim ulang data setelah konek
        client.send(dataStr);
        Serial.println("Terkirim (setelah konek ulang): " + dataStr);
      }
    }
  }

  // Wajib dipanggil di loop untuk menjaga koneksi websocket
  client.poll();
}