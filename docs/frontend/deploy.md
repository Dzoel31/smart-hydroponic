# Deploy Frontend-vue dengan Docker dan NGINX (Linux)

Dokumentasi ini menjelaskan cara membangun dan menjalankan frontend-vue sebagai aplikasi statis. Hasil build disajikan oleh NGINX di dalam container, lalu bisa juga diproxy oleh NGINX host jika diperlukan.

## Build Docker Image

Pastikan Anda berada di root repository, lalu build image dari folder `frontend-vue`:

```bash
docker build -t smart-hydroponic-frontend ./frontend-vue
```

## Jalankan Container

Setelah image selesai dibangun, jalankan container pada port 80:

```bash
docker run -d --name smart-hydroponic-frontend -p 8080:80 smart-hydroponic-frontend
```

Akses aplikasi melalui `http://localhost:8080` atau melalui domain server Anda jika sudah dipasang reverse proxy.

## Konfigurasi NGINX Host

Jika ingin menempatkan frontend di belakang NGINX host, gunakan proxy pass ke container atau ke service yang menayangkan file statis.

Contoh proxy ke container Docker:

```nginx
server {
        listen 80;
        server_name dashboard.example.com;

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

## Catatan Penting

- Build frontend berasal dari folder `frontend-vue`.
- Dockerfile frontend-vue sudah menyalin hasil build ke NGINX image.
- Jika aplikasi dipasang pada subpath, sesuaikan `base` di `vite.config.ts` sebelum build.

Jika Anda tetap memakai NGINX host, restart service setelah menyimpan konfigurasi:

```bash
sudo systemctl restart nginx
```

Contoh akses aplikasi setelah deploy: [http://123.123.123.123/dashboard-hidroponik](http://123.123.123.123/dashboard-hidroponik)
