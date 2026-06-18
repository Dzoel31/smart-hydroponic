# Glossary

## Tujuan Bagian Ini

Glossary berisi definisi singkat istilah teknis yang sering muncul di dokumentasi Smart Hydroponic.

## Istilah Umum

**IoT**
: Internet of Things, yaitu konsep menghubungkan perangkat fisik ke internet atau jaringan agar dapat mengirim dan menerima data.

**Sensor**
: Komponen yang membaca kondisi tertentu, misalnya suhu, kelembaban, pH, atau aliran air.

**Aktuator**
: Komponen yang melakukan aksi, misalnya pompa air, relay, motor, atau lampu.

**Mikrokontroler**
: Komputer kecil pada satu papan elektronik. Contohnya ESP32 dan ESP8266.

**Firmware**
: Program yang berjalan di mikrokontroler.

## Istilah Backend dan Frontend

**Backend**
: Bagian aplikasi yang berjalan di server. Backend menerima data, mengolah request, dan berkomunikasi dengan database.

**Frontend**
: Bagian aplikasi yang dilihat pengguna di browser. Dalam proyek ini frontend berupa dashboard Vue.

**API**
: Antarmuka agar aplikasi atau perangkat dapat saling bertukar data.

**REST API**
: API berbasis HTTP yang biasanya memakai endpoint seperti `GET`, `POST`, `PUT`, dan `DELETE`.

**WebSocket**
: Koneksi dua arah yang dapat terus terbuka, cocok untuk komunikasi real-time.

**CoAP**
: Protokol komunikasi ringan yang sering digunakan untuk perangkat IoT.

## Istilah Database

**PostgreSQL**
: Database relasional yang digunakan untuk menyimpan data proyek.

**TimescaleDB**
: Ekstensi PostgreSQL yang dirancang untuk data time-series.

**Time-series data**
: Data yang dicatat berdasarkan waktu, misalnya suhu setiap 10 detik.

**Migration**
: Perubahan struktur database yang dicatat dalam file agar dapat dijalankan berulang secara terkontrol.

**Pydantic**
: Library Python untuk memeriksa dan membentuk data yang masuk atau keluar dari API.

**SQLAlchemy**
: Library Python yang membantu aplikasi membaca dan menulis data ke database.

**Alembic**
: Alat untuk mencatat dan menjalankan perubahan struktur database.

## Istilah Deployment

**Docker**
: Alat untuk menjalankan aplikasi dalam container.

**Container**
: Lingkungan ringan yang berisi aplikasi dan dependency yang dibutuhkan.

**Docker Compose**
: Alat untuk menjalankan banyak container sekaligus, misalnya database, backend, dan frontend.

**NGINX**
: Web server atau reverse proxy yang meneruskan request dari pengguna ke aplikasi.

**Reverse proxy**
: Server perantara yang menerima request publik lalu meneruskannya ke service internal.

**Environment variable**
: Konfigurasi yang disimpan di luar kode, misalnya nama database, password, atau secret token.
