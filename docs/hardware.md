# Hardware dan Sensor

## Tujuan Bagian Ini

Bagian ini mengenalkan perangkat fisik yang digunakan pada Smart Hydroponic. Tujuannya agar pembaca tidak hanya tahu nama komponen, tetapi juga memahami peran setiap komponen dalam sistem.

## Mikrokontroler

### ESP32

ESP32 adalah mikrokontroler yang memiliki Wi-Fi dan performa cukup baik untuk membaca beberapa sensor sekaligus. Dalam proyek ini, ESP32 digunakan untuk membaca data tanaman atau lingkungan dan mengirim data tersebut ke backend.

### ESP8266

ESP8266 juga memiliki Wi-Fi dan sering digunakan untuk perangkat IoT sederhana. Dalam proyek ini, ESP8266 dapat digunakan untuk membaca data tertentu atau mengendalikan aktuator.

## Sensor

### DHT11

DHT11 membaca suhu dan kelembaban udara. Data ini membantu pengguna melihat kondisi lingkungan sekitar tanaman.

### Soil Moisture Sensor

Sensor ini membaca kelembaban media tanam. Walaupun hidroponik tidak memakai tanah seperti pertanian biasa, sensor kelembaban masih dapat digunakan pada media tanam tertentu.

### Water Flow Sensor

Water flow sensor membaca aliran air. Sensor ini membantu memastikan air atau nutrisi benar-benar mengalir di sistem.

### Ultrasonic Sensor

Ultrasonic sensor dapat digunakan untuk memperkirakan tinggi air pada wadah. Sensor ini bekerja dengan mengukur jarak antara sensor dan permukaan air.

### pH Sensor

pH sensor membaca tingkat keasaman larutan nutrisi. Nilai pH penting karena tanaman menyerap nutrisi dengan baik pada rentang pH tertentu.

### TDS Sensor

TDS sensor membaca jumlah zat terlarut dalam air. Dalam hidroponik, nilai ini sering dipakai untuk memperkirakan konsentrasi nutrisi.

## Aktuator

### Water Pump

Pompa air mengalirkan air atau nutrisi ke tanaman. Pompa biasanya dikendalikan melalui relay atau modul driver.

### Relay Module

Relay berfungsi seperti saklar elektronik. Backend atau mikrokontroler dapat memberi perintah agar relay menyalakan atau mematikan perangkat.

### Grow Light LED

Grow light LED membantu pencahayaan tanaman, terutama jika cahaya alami kurang.

## Komponen Pendukung

- **Breadboard** membantu membuat rangkaian sementara tanpa solder.
- **Jumper wires** menghubungkan pin antar komponen.
- **Power supply** harus disesuaikan dengan kebutuhan sensor, pompa, relay, dan mikrokontroler.
