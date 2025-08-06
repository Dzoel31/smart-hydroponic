# Cara Deploy Backend

Untuk melakukan deploy backend, ikuti langkah-langkah berikut:

1. **Clone Repository**

    Gunakan perintah berikut untuk meng-clone repository:

     ```
     git clone https://github.com/project-iot-fik/smart-hydroponic.git
     ```

2. **Masuk ke Direktori Backend**

    Setelah repository berhasil di-clone, masuk ke direktori backend dengan perintah berikut:

    ```
    cd smart-hydroponic/backend
    ```

3. **Install Dependencies**

    Pastikan Anda telah menginstall semua dependencies yang diperlukan. Gunakan perintah berikut:

    ```
    npm install
    ```

4. **Konfigurasi Environment Variables**

    Buat file `.env` di direktori backend dan tambahkan konfigurasi environment variables yang diperlukan:

    Pertama, buat copy dari file `.env.example`:

    ```
    cp .env.example .env
    ```

    Kemudian, edit file `.env` sesuai dengan kebutuhan proyek.

5. **Lakukan Migrasi Database**

    Berikut adalah beberapa perintah untuk melakukan migrasi database:

    - `npm run migrate:latest`: Untuk menjalankan migrasi terbaru.
    - `npm run migrate:rollback`: Untuk membatalkan migrasi terakhir.
    - `npm run migrate:clean`: Untuk membatalkan semua migrasi.

6. **Jalankan Server**

    Setelah semua konfigurasi selesai, jalankan server dengan perintah berikut:

    Jika masih dalam test mode, gunakan:

    ```
    npm run start
    ```

    Untuk mode produksi, gunakan:

    ```
    pm2 start server.js --name backend-iot-hydroponic
    ```

    kemudian pastikan PM2 berjalan dengan baik:

    ```
    pm2 list
    ```

    Anda dapat melihat status service backend anda pada output PM2.
