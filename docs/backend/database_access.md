# Cara Akses Database

## Tujuan Bagian Ini

Bagian ini menjelaskan cara masuk ke database PostgreSQL untuk mengecek data, melihat tabel, atau membantu proses debugging.

Gunakan akses database dengan hati-hati. Jangan mengubah atau menghapus data produksi tanpa arahan tim.

## Yang Perlu Disiapkan

1. PostgreSQL client, biasanya command `psql`.
2. Informasi koneksi database dari file `.env` atau dari admin server.
3. Akses jaringan ke database.

Informasi yang biasanya dibutuhkan:

- `PGHOST`: alamat database.
- `PGPORT`: port database.
- `PGDATABASE`: nama database.
- `PGUSER`: user database.
- `PGPASSWORD`: password database.

## Akses Dari Komputer Lokal

Format umum:

```bash
psql -h <host> -p <port> -U <username> -d <database_name>
```

Contoh lokal:

```bash
psql -h localhost -p 5432 -U admin_iot_db -d iot_hydroponik
```

Setelah command dijalankan, PostgreSQL akan meminta password.

## Akses Database Dari Docker Compose

Jika database berjalan lewat Docker Compose, Anda dapat masuk ke container database.

Jalankan dari root repository:

```bash
docker compose -f docker-compose.dev.yml exec db psql -U admin_iot_db -d iot_hydroponik
```

Sesuaikan user dan nama database dengan isi file `.env`.

## Command Dasar psql

Setelah masuk ke `psql`, beberapa command yang berguna:

```psql
\dt
```

Menampilkan daftar tabel.

```psql
\d nama_tabel
```

Menampilkan struktur tabel tertentu.

```psql
SELECT now();
```

Menguji query sederhana.

```psql
\q
```

Keluar dari `psql`.

## Cara Mengecek Berhasil

Akses database berhasil jika terminal berubah menjadi prompt PostgreSQL, misalnya:

```text
iot_hydroponik=>
```

## Jika Terjadi Error

- `connection refused`: database belum berjalan atau host/port salah.
- `password authentication failed`: username atau password salah.
- `database does not exist`: nama database salah atau belum dibuat.
- `psql is not recognized`: PostgreSQL client belum terinstall atau belum masuk PATH.
