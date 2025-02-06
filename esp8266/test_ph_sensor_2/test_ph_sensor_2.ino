#define SensorPin 27  // Pin sensor pH pada ESP32
#define ADC_MAX 4095  // Resolusi ADC ESP32 (12-bit)
#define VOLTAGE_REF 5.5  // Tegangan referensi ESP32
#define Calibration_Value 2.6

unsigned long int avgValue;
int buf[10], temp;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32 pH Sensor Ready");
}

void loop() {
  // Ambil 10 sampel dari sensor
  for (int i = 0; i < 20; i++) { 
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  avgValue = 0;
  for (int i = 0; i < 20; i++) {
    avgValue += buf[i];
  }
  avgValue /= 20;
  
  // Konversi nilai ADC ke tegangan
  float voltage = (float)avgValue * VOLTAGE_REF / ADC_MAX;
  
  // Konversi tegangan ke nilai pH
  float phValue = (3.5 * voltage) + Calibration_Value;
  
  // Tampilkan hasil di Serial Monitor
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V   pH: ");
  Serial.println(phValue, 2);
  
  // Blink LED sebagai indikator
  digitalWrite(13, HIGH);
  delay(800);
  digitalWrite(13, LOW);
  delay(1000);
}
