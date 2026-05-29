# Cara Membaca Dokumentasi Ini

## Tujuan Bagian Ini

Halaman ini membantu Anda memilih urutan belajar yang sesuai. Dokumentasi Smart Hydroponic berisi beberapa topik teknis, tetapi tidak semuanya harus dibaca sekaligus.

## Jika Anda Baru Pertama Kali

Gunakan urutan berikut:

1. Baca [Pengantar](index.md) untuk memahami tujuan proyek.
2. Baca [Memulai Sistem Smart Hydroponic](getting-started.md) untuk menyiapkan alat dan menjalankan service langsung.
3. Baca [Arsitektur Sistem](architecture.md) untuk memahami alur data.
4. Baca [Tech Stack](tech-stack.md) untuk mengenal fungsi teknologi yang dipakai.
5. Baca [Struktur Proyek](project-structure.md) agar tahu folder mana yang perlu dibuka.
6. Baca [Hardware dan Sensor](hardware.md) untuk mengenal perangkat fisik.
7. Buka [Troubleshooting](troubleshooting.md) jika ada error saat mencoba.

## Jika Anda Ingin Mengembangkan Backend

Baca urutan berikut:

1. [Pengembangan Backend](backend/development.md)
2. [Setup Database](backend/setup_database.md)
3. [Cara Akses Database](backend/database_access.md)
4. [Deploy Backend Python](backend/deploy_python.md)

Backend utama saat ini menggunakan Python dan FastAPI. Untuk dokumentasi backend, gunakan halaman yang ada pada bagian Backend di navigasi ini.

## Jika Anda Ingin Mengembangkan Frontend

Baca urutan berikut:

1. [Pengembangan Frontend Vue](frontend/development.md)
2. [Deploy Frontend](frontend/deploy.md)
3. [Troubleshooting](troubleshooting.md)

Frontend utama saat ini berada di folder `frontend-vue`.

## Cara Membaca Command

Setiap command biasanya ditulis dalam blok seperti ini:

```bash
docker compose -f docker-compose.dev.yml up -d
```

Perhatikan tiga hal sebelum menjalankan command:

1. **Lokasi folder**: beberapa command harus dijalankan dari root repository, beberapa dari folder `backend` atau `frontend-vue`.
2. **Tujuan command**: baca penjelasan singkat di atas command.
3. **Tanda berhasil**: cek output terminal, status container, halaman browser, atau endpoint health check.

## Jika Ada Istilah Yang Belum Dipahami

Buka [Glossary](glossary.md). Halaman itu berisi definisi singkat untuk istilah seperti IoT, backend, API, Docker, NGINX, dan TimescaleDB.
