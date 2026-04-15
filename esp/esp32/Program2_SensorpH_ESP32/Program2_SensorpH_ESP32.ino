#include <Arduino.h>

#define PH_SENSOR_PIN 34   // Pin ADC ESP32 untuk sensor pH

// Nilai pH standar larutan buffer
const float pH_low = 4.01;
const float pH_high = 9.18;

// Catat hasil tegangan (Volt) dari kalibrasi probe di buffer 4.01 dan 9.18
// GANTI nilai di bawah dengan hasil pengukuranmu
const float V_low = 3.15;   // contoh: tegangan saat di buffer pH 4.01
const float V_high = 2.05;  // contoh: tegangan saat di buffer pH 9.18

// Variabel untuk hasil kalibrasi
float slope, intercept;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Hitung slope dan intercept untuk konversi tegangan → pH
  slope = (pH_high - pH_low) / (V_high - V_low);
  intercept = pH_high - slope * V_high;

  Serial.println("Kalibrasi pH selesai:");
  Serial.print("Slope = "); Serial.println(slope, 6);
  Serial.print("Intercept = "); Serial.println(intercept, 6);
}

float readVoltageAverage(int pin, int samples = 10) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(30);
  }
  float raw = sum / (float)samples;
  return raw * 3.3 / 4095.0;  // konversi ADC → Volt (ESP32 12-bit, 0–3.3V)
}

void loop() {
  // Baca rata-rata tegangan dari sensor
  float volt = readVoltageAverage(PH_SENSOR_PIN);

  // Konversi ke pH
  float pH_value = slope * volt + intercept;

  // Cetak hasil
  Serial.print("Tegangan (V): ");
  Serial.print(volt, 3);
  Serial.print(" | pH: ");
  Serial.println(pH_value, 2);

  delay(1000);
}
