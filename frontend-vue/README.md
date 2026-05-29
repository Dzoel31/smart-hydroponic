# Frontend Vue Smart Hydroponic

Frontend adalah dashboard web yang digunakan pengguna untuk memantau data hidroponik dan mengakses fitur aplikasi dari browser.

Frontend utama saat ini menggunakan **Vue 3**, **TypeScript**, **Vite**, **Vue Router**, **Axios**, dan **Chart.js**.

## Tujuan Bagian Ini

README ini membantu Anda menjalankan dashboard dari folder `frontend-vue`, baik untuk development lokal maupun build produksi.

## Yang Perlu Disiapkan

1. Node.js dan npm.
2. Backend Smart Hydroponic yang sudah berjalan.
3. Browser untuk membuka dashboard.

## Install Dependency

Jalankan dari folder `frontend-vue`.

```bash
npm install
```

Command ini memasang library frontend berdasarkan `package.json`.

## Menjalankan Mode Development

Jalankan:

```bash
npm run dev
```

Tanda berhasil: terminal Vite menampilkan alamat lokal, biasanya `http://localhost:5173`.

## Build Produksi

Jalankan:

```bash
npm run build
```

Command ini melakukan pengecekan TypeScript dan membuat hasil build statis. Jika berhasil, hasil build akan berada di folder `dist`.

## Preview Hasil Build

Jalankan:

```bash
npm run preview
```

Gunakan command ini untuk mengecek hasil build sebelum deployment.

## Generate API Client

Frontend memakai API client hasil generate dari OpenAPI backend. Pastikan backend berjalan, lalu jalankan:

```bash
npm run generate-api
```

Command ini membaca OpenAPI schema dari:

```text
http://127.0.0.1:8000/smart-hydroponic/api/v2/openapi.json
```

Hasil generate berada di `src/api`.

## Konfigurasi Backend

Pastikan frontend mengarah ke alamat backend yang benar. Untuk development lokal, backend biasanya berjalan di:

```text
http://localhost:8000/smart-hydroponic/api/v2
```

Untuk mengecek apakah backend lokal hidup, buka:

```text
http://localhost:8000/health
```

Jika dashboard tidak menampilkan data, buka Developer Tools di browser dan cek tab Network untuk melihat apakah request API gagal.

## Jika Terjadi Error

- Jika `npm install` gagal, cek versi Node.js dan koneksi internet.
- Jika `npm run build` gagal, baca error TypeScript yang muncul di terminal.
- Jika data tidak muncul, pastikan backend dan database berjalan.
- Jika API client gagal digenerate, pastikan endpoint OpenAPI backend dapat dibuka.
