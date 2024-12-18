const int ph_Pin = 23;
float Po = 0;
float PH_step;
int nilai_analog_PH;
double TeganganPh;

//untuk kalibrasi
float PH4 = 3.08;
float PH7 = 2.45;

void setup() {
  pinMode (ph_Pin, INPUT);
  Serial.begin(9600);
}

void loop() {
  int nilai_analog_PH = analogRead(ph_Pin);
  Serial.print("Nilai ADC Ph: ");
  Serial.println(nilai_analog_PH);
  TeganganPh = 5 / 1024 * nilai_analog_PH;
  Serial.print("TeganganPh: ");
  Serial.println(TeganganPh, 2);

  PH_step = (PH4 - PH7) /3;
  Po = 7.00 + ((PH7 -TeganganPh) / PH_step);
  Serial.print("Nilai PH cairan:");
  Serial.println(Po, 2);
  delay(1000);
}
