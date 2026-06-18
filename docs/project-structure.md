# Struktur Proyek

## Tujuan Bagian Ini

Bagian ini menjelaskan fungsi folder utama dalam repository. Tujuannya bukan menghafal semua file, tetapi tahu harus membuka folder mana saat ingin mencari sesuatu.

## Peta Folder Utama

```mermaid
flowchart TD
    Root["smart_hydroponic/"] --> Backend["backend/<br/>API dan logika server"]
    Root --> Frontend["frontend-vue/<br/>Dashboard web"]
    Root --> ESP["esp/<br/>Program mikrokontroler"]
    Root --> Docs["docs/<br/>Dokumentasi belajar"]
    Root --> Monitoring["monitoring/<br/>Konfigurasi monitoring"]
    Root --> Compose["docker-compose.*.yml<br/>Menjalankan service"]
    Root --> Zensical["zensical.toml<br/>Navigasi dan konfigurasi dokumentasi"]
```

Gunakan diagram ini sebagai peta awal. Kalau ingin mengubah tampilan dashboard, buka `frontend-vue`. Kalau ingin mengubah API, buka `backend`.

## Folder `backend/`

Folder ini berisi backend Python/FastAPI.

```mermaid
flowchart TD
    Backend["backend/"] --> Main["main.py<br/>Aplikasi FastAPI dimulai"]
    Backend --> Routes["routes/<br/>Daftar endpoint API"]
    Backend --> Models["models/<br/>Bentuk tabel database"]
    Backend --> Schemas["schemas/<br/>Bentuk data request/response"]
    Backend --> Services["services/<br/>Logika aplikasi"]
    Backend --> Config["config/<br/>Konfigurasi database/aplikasi"]
    Backend --> Migrations["migrations/<br/>Riwayat perubahan database"]
    Backend --> Test["test/<br/>Test dan simulasi"]
```

Penjelasan singkat:

| Folder/File | Fungsi |
| --- | --- |
| `main.py` | Titik awal aplikasi FastAPI. |
| `routes/` | Tempat endpoint API didefinisikan. |
| `models/` | Tempat struktur tabel database direpresentasikan dalam kode. |
| `schemas/` | Tempat bentuk data request dan response didefinisikan. |
| `services/` | Tempat logika aplikasi yang lebih panjang atau berulang. |
| `config/` | Tempat konfigurasi seperti koneksi database. |
| `migrations/` | Tempat Alembic menyimpan perubahan struktur database. |
| `test/` | Tempat test dan simulasi perangkat. |

Alur kerja:

```mermaid
flowchart LR
    U["1. User / Frontend"] --> Routes["2. routes/"]
    Routes --> Req["3. schemas/ request"]
    Req --> Services["4. services/"]
    Services --> Models["5. models/"]
    Models --> DB["6. Database PostgreSQL + TimescaleDB"]

    DB --> Models
    Models --> Services
    Services --> Res["7. schemas/ response"]
    Res --> Routes
    Routes --> U

    Routes -. ambil data .-> Services
    Services -. query .-> Models
    Models -. baca/tulis .-> DB
```

## Folder `frontend-vue/`

Folder ini berisi dashboard Vue.

```mermaid
flowchart TD
    Frontend["frontend-vue/"] --> Views["src/views/<br/>Halaman dashboard"]
    Frontend --> Components["src/components/<br/>Bagian UI yang dipakai ulang"]
    Frontend --> Router["src/router/<br/>Pengaturan perpindahan halaman"]
    Frontend --> API["src/api/<br/>Client untuk memanggil backend"]
    Frontend --> Assets["src/assets/<br/>Gambar dan asset pendukung"]
```

Penjelasan singkat:

| Folder/File | Fungsi |
| --- | --- |
| `src/views/` | Halaman utama seperti dashboard, login, atau analytics. |
| `src/components/` | Komponen tampilan seperti sidebar, topbar, footer, atau modal. |
| `src/router/` | Mengatur URL dan halaman yang ditampilkan. |
| `src/api/` | Kode generated untuk memanggil backend API. |
| `src/assets/` | Gambar, style, dan asset tampilan. |

Alur kerja:

```mermaid
flowchart LR
    U1["1. User"] --> R2["2. Router"]
    R2 --> V3["3. Views"]
    V3 <--> C4["4. Components"]
    V3 --> A5["5. API Client"]
    A5 --> B6["6. Backend API"]
    B6 <--> D7["7. Database"]
```

## Folder `esp/`

Folder ini berisi program untuk ESP32 dan ESP8266. Program di folder ini adalah firmware, yaitu kode yang dijalankan pada mikrokontroler.

Beberapa subfolder memisahkan eksperimen atau protokol komunikasi, misalnya WebSocket dan CoAP.

## Folder `docs/`

Folder ini berisi dokumentasi yang sedang Anda baca. File `zensical.toml` mengatur urutan halaman dan konfigurasi situs agar dokumentasi dapat dibaca seperti buku panduan.

## File Docker Compose

File seperti `docker-compose.dev.yml` dan `docker-compose.prod.yml` membantu menjalankan beberapa service sekaligus.

Contohnya:

```text
database + backend + frontend
```

Dengan Docker Compose, Anda tidak perlu menjalankan semuanya satu per satu secara manual.

## Cara Agar Tidak Overwhelm

Saat baru belajar, jangan buka semua folder sekaligus. Mulai dari kebutuhan (hal yang membuat kamu penasaran) lalu buka folder yang relevan. Misalnya:

1. Ingin tahu alur sistem: buka `docs/`.
2. Ingin menjalankan aplikasi: buka `docker-compose.dev.yml` dan [Memulai Sistem Smart Hydroponic](getting-started.md).
3. Ingin memahami API: buka `backend`. Baca `main.py` untuk melihat bagaimana aplikasi dimulai, lalu buka `routes/` untuk melihat endpoint API. Buka `schemas/` untuk melihat bentuk data yang dikirim dan diterima. Setiap file saling terhubung satu sama lain, jadi jangan khawatir kalau belum langsung paham. Baca perlahan dan coba ikuti alur data dari user ke database dan kembali lagi.
4. Ingin memahami tampilan: buka `frontend-vue/src/views/` atau `frontend-vue/src/components/`.
5. Ingin melihat program perangkat: buka `esp/`.
