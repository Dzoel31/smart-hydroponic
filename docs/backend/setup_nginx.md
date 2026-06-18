# Setup NGINX

## Tujuan Bagian Ini

NGINX digunakan sebagai reverse proxy. Artinya, pengguna mengakses domain atau IP server melalui NGINX, lalu NGINX meneruskan request ke backend atau frontend yang berjalan di port internal.

Contoh:

```text
Browser -> NGINX port 80 -> Backend port 8000
```

## Yang Perlu Disiapkan

1. Server Linux.
2. NGINX.
3. Backend atau frontend yang sudah berjalan.
4. Domain atau IP server. Contoh: `192.168.1.10` atau `hydroponic.example.com`.

Jika server diakses dari laptop, login terlebih dahulu melalui [SSH](deploy_ssh.md), lalu jalankan command NGINX di dalam server.

## Instalasi NGINX

Jalankan di server:

```bash
sudo apt update
sudo apt-get install nginx
```

## Membuat Konfigurasi Situs

Buat file konfigurasi:

```bash
sudo nano /etc/nginx/sites-available/iot-hidroponik
```

Contoh konfigurasi untuk meneruskan request ke backend pada port `8000`:

```nginx
server {
    listen 80;
    server_name hydroponic.example.com;

    location / {
        proxy_pass http://localhost:8000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }
}
```

Ganti `hydroponic.example.com` dengan domain atau IP server Anda.

## Penjelasan Singkat

- `listen 80` berarti NGINX menerima request HTTP pada port 80.
- `server_name` berisi domain atau IP yang digunakan pengguna.
- `proxy_pass` adalah alamat aplikasi tujuan di dalam server.
- Header `Upgrade` dan `Connection` membantu komunikasi WebSocket.
- Header `X-Forwarded-*` meneruskan informasi request asli ke aplikasi.

## Mengaktifkan Konfigurasi

Buat symlink ke `sites-enabled`:

```bash
sudo ln -s /etc/nginx/sites-available/iot-hidroponik /etc/nginx/sites-enabled/
```

Uji konfigurasi:

```bash
sudo nginx -t
```

Jika hasilnya `ok` atau `successful`, restart NGINX:

```bash
sudo systemctl restart nginx
```

## Cara Mengecek Berhasil

1. Cek status NGINX.

   ```bash
   sudo systemctl status nginx
   ```

2. Buka domain atau IP server dari browser.
3. Jika memakai backend, buka endpoint health check melalui domain.

## Jika Terjadi Error

- `502 Bad Gateway`: aplikasi tujuan belum berjalan atau `proxy_pass` salah.
- `nginx -t` gagal: ada kesalahan sintaks konfigurasi.
- Domain tidak terbuka: cek DNS, firewall, dan apakah port 80 dibuka.
- WebSocket gagal: pastikan header `Upgrade` dan `Connection` ada.
