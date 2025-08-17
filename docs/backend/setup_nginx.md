# Setup NGINX (Linux)

NGINX dapat digunakan sebagai web server dan reverse proxy untuk aplikasi web. NGINX bekerja sebagai proxy yaitu dengan menerima permintaan dari klien kemudian meneruskannya ke server aplikasi yang sesuai. Misalkan, aplikasi berjalan di ip 123.123.123.123, sedangkan aplikasi yang berjalan di server pada localhost:15000, maka NGINX akan meneruskan permintaan dari klien ke localhost:15000.

Untuk mengatur NGINX, dapat mengikuti langkah-langkah berikut:

## Instalasi NGINX

Untuk menginstal NGINX, gunakan perintah berikut:

```
sudo apt update
sudo apt-get install nginx
```

## Konfigurasi NGINX

Setelah NGINX terinstal, konfigurasi dapat dilakukan dengan mengedit file konfigurasi utama yang biasanya terletak di `/etc/nginx/nginx.conf`. Namun untuk konfigurasi situs tertentu, Anda dapat membuat file baru di direktori `/etc/nginx/sites-available/` dan membuat symlink ke direktori `/etc/nginx/sites-enabled/`.

## Konfigurasi untuk proyek Smart Hydroponic

File konfigurasi untuk proyek Smart Hydroponic berada di path `/etc/nginx/sites-available/iot-hidroponik` dan dapat dibuat dengan perintah berikut:

```
sudo nano /etc/nginx/sites-available/iot-hidroponik
```

Isi file konfigurasi tersebut dengan konten berikut:

```
server {
    listen 80; # Port default yang digunakan untuk HTTP
    server_name your_domain_or_ip; # Ganti dengan domain atau IP Anda. IP bisa cek dengan `ip a`

    location / {
        proxy_pass http://localhost:15000;  # Ganti dengan port aplikasi Anda
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
}
```

Penjelasan konfigurasi:

- `listen 80;` - NGINX akan mendengarkan pada port 80.
- `server_name your_domain_or_ip;` - Ganti dengan domain atau alamat IP server.
- `location /` - Mengatur lokasi root untuk server.
- `proxy_pass http://localhost:15000;` - Mengarahkan permintaan ke aplikasi yang berjalan pada port 15000 (ganti sesuai dengan port aplikasi Anda).
- `proxy_set_header` - Mengatur header yang diperlukan untuk koneksi proxy.
- `proxy_set_header Upgrade $http_upgrade;` - Memungkinkan WebSocket untuk berfungsi dengan baik.
- `proxy_set_header Connection 'upgrade';` - Menangani koneksi upgrade. Biasanya diperlukan untuk WebSocket.
- `proxy_set_header Host $host;` - Mengatur header Host.
- `proxy_cache_bypass $http_upgrade;` - Melewati cache jika ada upgrade.

## Aktifkan Konfigurasi

Setelah file konfigurasi dibuat, aktifkan dengan membuat symlink ke direktori `sites-enabled`:

```
sudo ln -s /etc/nginx/sites-available/iot-hidroponik /etc/nginx/sites-enabled/
```

## Uji Konfigurasi NGINX

Setelah konfigurasi selesai, uji konfigurasi NGINX untuk memastikan tidak ada kesalahan:

```
sudo nginx -t
```

Jika tidak ada kesalahan, Anda akan melihat pesan yang menyatakan bahwa konfigurasi NGINX `ok` atau `successful`. Setelah itu, restart NGINX untuk menerapkan perubahan:

```
sudo systemctl restart nginx
```

## Cek Status NGINX

Untuk memastikan NGINX berjalan dengan baik dapat memeriksa statusnya dengan perintah berikut:

```
sudo systemctl status nginx
```

Jika NGINX berjalan dengan baik, Anda akan melihat status `active (running)`.

## Akses Aplikasi

Akses aplikasi dengan membuka browser dan mengunjungi `http://your_domain_or_ip`.
Jika semuanya berjalan dengan benar, maka akan muncul halaman aplikasi Smart Hydroponic.

