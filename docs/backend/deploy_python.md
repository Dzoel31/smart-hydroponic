# Deploy Backend Python

## Tujuan Bagian Ini

Bagian ini menjelaskan cara menjalankan backend Python/FastAPI menggunakan Docker. Cara ini cocok untuk server karena dependency aplikasi dan environment lebih mudah dikontrol.

## Yang Perlu Disiapkan

1. Server Linux atau komputer yang sudah memiliki Docker.
2. File `.env` yang sudah diisi.
3. Akses ke repository proyek.
4. Port backend yang dapat diakses, misalnya `8000`.

Jika deployment dilakukan di VPS atau server jarak jauh, pastikan Anda sudah bisa masuk ke server melalui [SSH](deploy_ssh.md) terlebih dahulu.

## Langkah-Langkah

Jalankan dari root repository.

1. Clone repository.

   ```bash
   git clone https://github.com/IoT-Smart-Hydroponic/smart-hydroponic.git
   ```

2. Masuk ke folder proyek.

   ```bash
   cd smart-hydroponic
   ```

3. Siapkan file `.env`.

   ```bash
   cp .env.example .env
   ```

   Isi nilai database, secret JWT, dan konfigurasi lain sesuai server.

4. Jalankan service development atau produksi.

   Untuk development:

   ```bash
   docker compose -f docker-compose.dev.yml up -d
   ```

   Untuk produksi:

   ```bash
   docker compose -f docker-compose.prod.yml up -d
   ```

5. Cek status container.

   ```bash
   docker compose -f docker-compose.prod.yml ps
   ```

6. Cek log aplikasi.

   ```bash
   docker compose -f docker-compose.prod.yml logs -f api
   ```

7. Buka health check.

   ```text
   http://alamat-server:8000/health
   ```

   Ganti `alamat-server` dengan domain atau IP server, misalnya `192.168.1.10` atau `hydroponic.example.com`.

   Jika server memakai NGINX dengan prefix `/smart-hydroponic/api/v2`, URL publik dapat menjadi:

   ```text
   http://alamat-server/smart-hydroponic/api/v2/health
   ```

## Cara Mengecek Berhasil

Backend dianggap berhasil dideploy jika:

1. Container `hydroponic-backend` berjalan.
2. Health check mengembalikan status sehat.
3. Backend dapat mengakses database.
4. Dashboard dapat mengirim request ke backend.

## Menghentikan Aplikasi

Untuk menghentikan service produksi:

```bash
docker compose -f docker-compose.prod.yml down
```

Untuk menjalankan kembali:

```bash
docker compose -f docker-compose.prod.yml up -d
```

## Jika Terjadi Error

- Jika container `api` berhenti, cek log `api`.
- Jika database belum sehat, cek log `db`.
- Jika backend tidak bisa diakses dari luar server, cek firewall, security group, dan konfigurasi NGINX.
- Jika URL memakai domain, pastikan DNS sudah mengarah ke IP server.
