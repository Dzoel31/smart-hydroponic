# Deploy Frontend dengan NGINX (Linux)

Secara umum, alur untuk melakukan deploy frontend menggunakan NGINX mirip dengan backend (dapat dilihat pada [Setup NGINX (Linux)](../backend/setup_nginx.md)). NGINX akan berfungsi sebagai web server yang menyajikan file statis dari aplikasi frontend. Pada guide ini langsung melakukan setup untuk deploy frontend aplikasi Smart Hydroponic.

## Salin File Aplikasi Frontend

Salin file aplikasi frontend Anda ke direktori yang sesuai di server. Biasanya berada di folder `/var/www/`. Kemudian buat folder baru untuk aplikasi Smart Hydroponic:

```
sudo mkdir -p /var/www/Dashboard-iot-Hidroponik
```

Setelah itu, salin file aplikasi frontend ke dalam folder tersebut. Gunakan perintah berikut:

```
sudo cp -r /path/to/your/frontend/* /var/www/Dashboard-iot-Hidroponik/
```

## Konfigurasi NGINX untuk Frontend

Buat file konfigurasi NGINX untuk aplikasi frontend atau gunakan file yang sama dengan yang digunakan untuk backend.

```
sudo nano /etc/nginx/sites-available/iot-hidroponik
```

Kemudian tambahkan konfigurasi berikut:

location /dashboard-hidroponik {
                alias /var/www/Dashboard-iot-Hidroponik/;
                index index.html;
                try_files $uri $uri/ =404;
        }
Simpan dan keluar dari editor.

Penjelasan konfigurasi:

- `location /dashboard-hidroponik` - Mengatur lokasi untuk aplikasi frontend. (Diakses melalui URL `/dashboard-hidroponik`)
- `alias /var/www/Dashboard-iot-Hidroponik/;` - Menunjukkan direktori tempat file frontend disimpan.
- `index index.html;` - Menentukan file indeks yang akan disajikan.
- `try_files $uri $uri/ =404;` - Mencoba untuk menyajikan file yang diminta, jika tidak ditemukan, akan mengembalikan error 404.

## Aktifkan Konfigurasi NGINX

Setelah file konfigurasi dibuat, aktifkan dengan membuat symlink ke direktori `sites-enabled`:

```
sudo ln -s /etc/nginx/sites-available/iot-hidroponik /etc/nginx/sites-enabled/
```

Jika sudah ada symlink sebelumnya, perubahan yang ada di sites-available akan otomatis diterapkan.

## Uji Konfigurasi NGINX

Setelah konfigurasi selesai, uji konfigurasi NGINX untuk memastikan tidak ada kesalahan:

```
sudo nginx -t
```

Jika tidak ada kesalahan, Anda akan melihat pesan yang menyatakan bahwa konfigurasi NGINX valid. Setelah itu, restart NGINX untuk menerapkan perubahan:

```
sudo systemctl restart nginx
```

## Akses Aplikasi Frontend

Setelah NGINX berhasil di-restart, Anda dapat mengakses aplikasi frontend melalui URL, sebagai contoh:

http://123.123.123.123/dashboard-hidroponik

