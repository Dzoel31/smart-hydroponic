# QoS Evaluation: Skenario 2 dan Skenario 3

Dokumen ini menjelaskan kode pendukung untuk pengujian QoS berbasis WebSocket pada:

- Skenario 2: transmisi data inter-node dari sensor ke aktuator melalui server.
- Skenario 3: kendali manual dari dashboard ke aktuator melalui server.

Pengujian CoAP tetap dapat digunakan untuk Skenario 1. Untuk Skenario 2 dan 3,
implementasi saat ini memakai WebSocket karena server perlu meneruskan pesan secara
push ke ESP8266 Actuator dan menerima ACK balik dari perangkat.

## Komponen Kode

| File | Fungsi |
| --- | --- |
| `backend/routes/hydroponic_routes.py` | Menerima data node, membuat ACK, meneruskan data ke aktuator, dan menghitung latency konfirmasi. |
| `backend/utils/evaluation_tracker.py` | Menyimpan pesan pending berdasarkan `correlation_id` atau `command_id` sampai ACK dari aktuator diterima. |
| `backend/schemas/hydroponic.py` | Menyediakan response `HydroponicControlResult` untuk hasil kendali dashboard. |
| `esp/esp8266/aktuator/aktuator.ino` | Firmware aktuator utama yang mengirim `actuator_ack` setelah menerima command atau sensor forward. |
| `backend/eval/esp/websocket/esp8266/esp8266.ino` | Firmware evaluasi WebSocket aktuator dengan format ACK yang sama. |

## Skenario 2: Sensor ke Aktuator melalui Server

Alur:

1. ESP32 Plant mengirim data ke `/hydroponics/ws/sensor-data`.
2. ESP32 Environment mengirim data ke `/hydroponics/ws/environment-data`.
3. ESP8266 Actuator mengirim status periodik ke `/hydroponics/ws/actuator-data`.
4. Server menggabungkan data menjadi snapshot.
5. Server membuat `correlation_id`, lalu mengirim pesan `sensor_forward` ke ESP8266.
6. ESP8266 memproses pesan dan membalas dengan `actuator_ack`.
7. Server meneruskan hasil ACK kembali ke ESP32 pengirim sebagai `inter_node_ack`.

Payload dari server ke ESP8266:

```json
{
  "type": "sensor_forward",
  "correlation_id": "sensor-sensor-12-uuid",
  "source_role": "sensor",
  "source_seq": 12,
  "forward_timestamp": 1710000000.123,
  "payload": {
    "moisture_avg": 65.2,
    "temperature_avg": 28.4,
    "pump_status": false,
    "light_status": true,
    "automation_status": true,
    "seq": 12,
    "arrival_timestamp": 1710000000.100
  }
}
```

ACK dari ESP8266 ke server:

```json
{
  "type": "actuator_ack",
  "ack_type": "inter_node_forward",
  "correlation_id": "sensor-sensor-12-uuid",
  "command_id": "sensor-sensor-12-uuid",
  "pump_status": false,
  "light_status": true,
  "automation_status": true,
  "device_id": "esp8266-actuator-device",
  "ack_time_ms": 123456
}
```

ACK dari server kembali ke ESP32 pengirim:

```json
{
  "status": "inter_node_ack",
  "correlation_id": "sensor-sensor-12-uuid",
  "seq": 12,
  "started_at": 1710000000.123,
  "ended_at": 1710000000.180,
  "latency_ms": 57.0,
  "actuator_response": {}
}
```

Nilai `latency_ms` pada pesan ini adalah latency forwarding server ke aktuator
sampai ACK aktuator diterima server. Jika ESP32 ingin menghitung round-trip dari
sudut pandangnya, ESP32 dapat menghitung waktu dari data sensor dikirim sampai
pesan `inter_node_ack` diterima.

## Skenario 3: Dashboard ke Aktuator

Alur:

1. Dashboard memanggil `POST /hydroponics/control`.
2. Server membuat `command_id` dan mencatat `time_start`.
3. Server meneruskan command ke ESP8266 Actuator.
4. ESP8266 menjalankan perubahan relay dan membalas `actuator_ack`.
5. Server mencatat `time_end`, menghitung `latency_ms`, lalu mengembalikan hasil ke dashboard.

Request dashboard:

```json
{
  "pump_status": true,
  "light_status": false,
  "automation_status": false
}
```

Payload dari server ke ESP8266:

```json
{
  "type": "command",
  "command_id": "dashboard-uuid",
  "time_start": 1710000000.123,
  "payload": {
    "pump_status": true,
    "light_status": false,
    "automation_status": false
  }
}
```

Response endpoint `/hydroponics/control` jika ACK diterima:

```json
{
  "pump_status": true,
  "light_status": false,
  "automation_status": false,
  "command_id": "dashboard-uuid",
  "confirmed": true,
  "time_start": 1710000000.123,
  "time_end": 1710000000.170,
  "latency_ms": 47.0,
  "actuator_response": {}
}
```

Jika ESP8266 tidak mengirim ACK dalam 5 detik, server tetap mengembalikan response
dengan `confirmed: false` dan `latency_ms: null`. Ini memudahkan pencatatan timeout
atau kegagalan eksekusi command.

## Catatan Pengukuran

- `seq` digunakan untuk korelasi data sensor dan deteksi packet loss pada sisi aplikasi.
- `correlation_id` digunakan untuk Skenario 2.
- `command_id` digunakan untuk Skenario 3.
- Log `[METRIC]` pada ESP tetap berguna untuk RTT node-server-node.
- Field `latency_ms` dari server pada Skenario 2 dan 3 merepresentasikan latency
  dari server mengirim pesan ke aktuator sampai server menerima ACK aktuator.
- Untuk membandingkan CoAP dan WebSocket secara adil, jalankan protokol secara
  terpisah dengan payload, interval, durasi, dan kondisi jaringan yang sama.
