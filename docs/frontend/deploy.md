# Deploy Frontend Vue Dengan Docker dan NGINX

## Tujuan Bagian Ini

Bagian ini menjelaskan cara membangun dan menjalankan dashboard Vue sebagai aplikasi statis. Hasil build frontend akan disajikan oleh NGINX di dalam container.

## Yang Perlu Disiapkan

1. Docker.
2. Folder `frontend-vue`.
3. Backend yang dapat diakses dari frontend.
4. Domain atau IP server jika deployment dilakukan di server.

## Build Docker Image

Jalankan dari root repository:

```bash
docker build -t smart-hydroponic-frontend ./frontend-vue
```

Command ini membuat image Docker berisi hasil build frontend dan NGINX.

## Menjalankan Container

Jalankan:

```bash
docker run -d --name smart-hydroponic-frontend -p 8080:80 smart-hydroponic-frontend
```

Artinya, port `8080` di komputer atau server akan meneruskan request ke port `80` di dalam container.

## Cara Mengecek Berhasil

Buka:

```text
http://localhost:8080
```

Jika di server, ganti `localhost` dengan IP atau domain server.

## Konfigurasi NGINX Host

Jika frontend ingin diakses melalui domain, NGINX host dapat meneruskan request ke container frontend.

Contoh:

```nginx
server {
    listen 80;
    server_name dashboard.hydroponic.example.com;

    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

Ganti `dashboard.hydroponic.example.com` dengan domain atau IP yang digunakan.

Setelah menyimpan konfigurasi, uji dan restart NGINX:

```bash
sudo nginx -t
sudo systemctl restart nginx
```

## Jika Aplikasi Dipasang Pada Subpath

Jika dashboard dipasang pada subpath seperti:

```text
http://server/dashboard-hidroponik
```

sesuaikan konfigurasi `base` di `vite.config.ts` sebelum build.

## Jika Terjadi Error

- Jika halaman putih, buka Developer Tools dan cek error JavaScript.
- Jika data tidak muncul, pastikan URL backend dapat diakses dari browser.
- Jika NGINX menampilkan 502, cek apakah container frontend berjalan.
- Jika aset tidak ditemukan pada subpath, cek konfigurasi `base` Vite.
