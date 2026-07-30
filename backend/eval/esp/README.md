# Firmware Evaluasi ESP

Folder ini memisahkan firmware evaluasi berdasarkan skenario supaya payload,
log Serial, dan definisi latency tidak tumpang tindih.

## Struktur

```text
backend/eval/esp/
  scenario_1_node_to_server/
    websocket/
    coap/
  scenario_2_sensor_to_actuator/
    websocket/
    coap/
  scenario_3_dashboard_to_actuator/
    websocket/
    coap/
```

Folder lama `websocket/`, `coap/`, dan `dummy/` dibiarkan sebagai referensi awal.
Untuk eksperimen skripsi, gunakan folder `scenario_*`.

## Skenario 1

Gunakan firmware di `scenario_1_node_to_server`.

Tujuan:

- Plant, Environment, dan Actuator mengirim data ke server.
- Node mencatat latency dari waktu kirim sampai ACK server diterima.

Log utama:

```text
[TX] Seq: <nomor sequence yang benar-benar berhasil dikirim>
[METRIC] atau [S1_METRIC]
```

Untuk eksperimen satu jam, jalankan serial logger dengan batas durasi agar window
observasi konsisten tanpa perlu merestart perangkat:

```text
python backend/tools/qos/serial_metric_logger.py --ports COM4 COM7 COM11 --labels plant environment actuator --duration 3600 --output scenario_1.csv
```

Sequence number boleh melanjutkan nilai sebelumnya. Jika perangkat restart saat
eksperimen, nilai sequence akan kembali ke awal dan analyzer harus memisahkannya
sebagai sesi baru. Gunakan jumlah log `[TX]` unik sebagai penyebut packet loss
aplikasi dan jumlah ACK `[METRIC]` unik sebagai pembilang pesan yang diterima.

## Skenario 2

Gunakan firmware di `scenario_2_sensor_to_actuator`.

Tujuan:

- Sensor mengirim data ke server.
- Server meneruskan hasil snapshot ke ESP8266 Actuator.
- Actuator mengirim ACK.
- Sensor mencatat latency end-to-end sampai ACK inter-node diterima.

Log utama:

```text
[S2_METRIC]
```

Pada WebSocket, ESP32 membaca pesan:

```json
{ "status": "inter_node_ack" }
```

Pada CoAP, ESP32 membaca response server yang memiliki:

```json
{ "coap_forward_ack": { "confirmed": true } }
```

## Skenario 3

Gunakan firmware di `scenario_3_dashboard_to_actuator`.

Tujuan:

- Dashboard mengirim command ke server.
- Server meneruskan command ke ESP8266 Actuator via WebSocket atau CoAP.
- Actuator menjalankan command dan mencetak ACK command.

Log utama:

```text
[S3_METRIC]
```

Latency utama Skenario 3 dihitung di backend dari `time_start` sampai response
aktuator diterima. Log Serial aktuator dipakai sebagai bukti bahwa command
diterima dan relay state sudah diproses.
