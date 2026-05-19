# QoS Evaluation: Skenario 2 dan Skenario 3

Dokumen ini menjelaskan kode pendukung untuk pengujian QoS berbasis WebSocket
dan CoAP pada:

- Skenario 2: transmisi data inter-node dari sensor ke aktuator melalui server.
- Skenario 3: kendali manual dari dashboard ke aktuator melalui server.

Untuk WebSocket, server meneruskan pesan melalui koneksi persistent berdasarkan
role `actuator`. Untuk CoAP, backend bertindak sebagai CoAP client yang mengirim
`PUT` langsung ke endpoint CoAP milik ESP8266 Actuator.

## Komponen Kode

| File | Fungsi |
| --- | --- |
| `backend/routes/hydroponic_routes.py` | Menerima data node, membuat ACK, meneruskan data ke aktuator, dan menghitung latency konfirmasi. |
| `backend/routes/coap_handler.py` | Menerima data CoAP dari node, membuat ACK, dan meneruskan snapshot ke aktuator via CoAP. |
| `backend/utils/evaluation_tracker.py` | Menyimpan pesan pending berdasarkan `correlation_id` atau `command_id` sampai ACK dari aktuator diterima. |
| `backend/utils/coap_actuator_client.py` | CoAP client backend untuk mengirim command atau sensor forward ke ESP8266 Actuator. |
| `backend/schemas/hydroponic.py` | Menyediakan response `HydroponicControlResult` untuk hasil kendali dashboard. |
| `esp/esp8266/aktuator/aktuator.ino` | Firmware aktuator utama yang mengirim `actuator_ack` setelah menerima command atau sensor forward. |
| `backend/eval/esp/websocket/esp8266/esp8266.ino` | Firmware evaluasi WebSocket aktuator dengan format ACK yang sama. |
| `backend/eval/esp/coap/esp8266/esp8266.ino` | Firmware evaluasi CoAP aktuator yang membuka endpoint `actuator/control`. |

## Struktur Firmware Per Skenario

Firmware eksperimen dipisahkan di `backend/eval/esp/scenario_*` agar setiap
skenario memiliki definisi log dan latency yang jelas.

```text
backend/eval/esp/
  scenario_1_node_to_server/
  scenario_2_sensor_to_actuator/
  scenario_3_dashboard_to_actuator/
```

Gunakan folder `scenario_1_node_to_server` untuk pengujian node-server,
`scenario_2_sensor_to_actuator` untuk pengujian sensor-server-actuator, dan
`scenario_3_dashboard_to_actuator` untuk pengujian command dashboard ke aktuator.
Folder lama `websocket/`, `coap/`, dan `dummy/` tetap tersedia sebagai referensi
awal.

## Konfigurasi CoAP Aktuator

Backend mengirim request CoAP ke ESP8266 Actuator melalui helper
`backend/utils/coap_actuator_client.py`.

Default target:

```text
coap://172.25.21.236:5683/actuator/control
```

Nilai tersebut bisa diganti dengan environment variable:

| Variable | Default | Fungsi |
| --- | --- | --- |
| `COAP_ACTUATOR_HOST` | `172.25.21.236` | IP ESP8266 Actuator yang menjalankan CoAP server. |
| `COAP_ACTUATOR_PORT` | `5683` | Port lokal CoAP server di ESP8266. |
| `COAP_ACTUATOR_CONTROL_PATH` | `actuator/control` | Path endpoint command/forwarding aktuator. |

## Skenario 2 WebSocket: Sensor ke Aktuator melalui Server

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

## Skenario 2 CoAP: Sensor ke Aktuator melalui Server

Alur:

1. ESP32 Plant mengirim data CoAP ke `/coap/hydroponics/plant`.
2. ESP32 Environment mengirim data CoAP ke `/coap/hydroponics/environment`.
3. ESP8266 Actuator mengirim status CoAP ke `/coap/hydroponics/actuator`.
4. Server menggabungkan data menjadi snapshot.
5. Server membuat `correlation_id`, lalu mengirim `PUT` CoAP ke ESP8266 pada endpoint `actuator/control`.
6. ESP8266 memproses payload, mengubah relay bila diperlukan, lalu membalas response CoAP berisi `actuator_ack`.
7. Server memasukkan hasil ACK aktuator ke response CoAP untuk node pengirim pada field `coap_forward_ack`.

Payload dari server ke ESP8266 CoAP:

```json
{
  "type": "sensor_forward",
  "correlation_id": "coap-sensor-sensor-12-1710000000123",
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

Response ESP8266 ke server:

```json
{
  "type": "actuator_ack",
  "ack_type": "coap_control",
  "correlation_id": "coap-sensor-sensor-12-1710000000123",
  "pump_status": false,
  "light_status": true,
  "automation_status": true,
  "device_id": "esp8266-actuator-device",
  "ack_time_ms": 123456
}
```

Response server ke ESP32 pengirim:

```json
{
  "status": "ack",
  "seq": 12,
  "arrival_timestamp": 1710000000.100,
  "coap_forward_ack": {
    "confirmed": true,
    "uri": "coap://172.25.21.236:5683/actuator/control",
    "started_at": 1710000000.123,
    "ended_at": 1710000000.180,
    "latency_ms": 57.0,
    "response_code": "2.05 Content",
    "actuator_payload": {}
  }
}
```

Nilai `coap_forward_ack.latency_ms` adalah durasi dari backend mengirim `PUT`
CoAP ke ESP8266 sampai response CoAP diterima kembali oleh backend.

Untuk memastikan forwarding terjadi saat pengujian, jalankan aktuator terlebih
dahulu agar status aktuator sudah tersedia di buffer aggregator, lalu jalankan
ESP32 Plant dan ESP32 Environment. Jika snapshot belum lengkap, response CoAP
tetap berisi ACK node-server biasa dengan `coap_forward_ack: null`.

## Skenario 3 WebSocket: Dashboard ke Aktuator

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

## Skenario 3 CoAP: Dashboard ke Aktuator

Endpoint dashboard yang sama dapat memakai CoAP dengan query parameter:

```text
POST /hydroponics/control?transport=coap
```

Alur:

1. Dashboard mengirim command HTTP ke backend dengan `transport=coap`.
2. Backend mencatat `time_start` dan membuat `command_id`.
3. Backend mengirim `PUT` CoAP ke ESP8266 Actuator endpoint `actuator/control`.
4. ESP8266 menjalankan command lalu membalas response CoAP berisi `actuator_ack`.
5. Backend mencatat `time_end`, menghitung `latency_ms`, dan mengembalikan hasil ke dashboard.

Request dashboard:

```json
{
  "pump_status": true,
  "light_status": false,
  "automation_status": false
}
```

Payload CoAP dari backend ke ESP8266:

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

Response HTTP ke dashboard:

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
  "actuator_response": {
    "confirmed": true,
    "uri": "coap://172.25.21.236:5683/actuator/control",
    "response_code": "2.05 Content",
    "actuator_payload": {}
  }
}
```

## Catatan Pengukuran

- `seq` digunakan untuk korelasi data sensor dan deteksi packet loss pada sisi aplikasi.
- `correlation_id` digunakan untuk Skenario 2.
- `command_id` digunakan untuk Skenario 3.
- Log `[METRIC]` pada ESP tetap berguna untuk RTT node-server-node.
- Field `latency_ms` dari server pada Skenario 2 dan 3 merepresentasikan latency
  dari server mengirim pesan ke aktuator sampai server menerima ACK/response aktuator.
- Pada WebSocket, ACK aktuator dikirim sebagai pesan WebSocket `actuator_ack`.
- Pada CoAP, ACK aktuator dikirim sebagai payload response CoAP dari endpoint `actuator/control`.
- Untuk membandingkan CoAP dan WebSocket secara adil, jalankan protokol secara
  terpisah dengan payload, interval, durasi, dan kondisi jaringan yang sama.
