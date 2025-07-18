# Backend Service untuk Aplikasi Smart Hydroponic

## Steps to Run the Backend Service

1. **Clone the Repository**
2. **Navigate to the Backend Directory**

   ```bash
    cd smart-hydroponic/backend
   ```

3. **Install Dependencies**

    ```bash
    npm install
    ```

4. **Set Up Environment Variables**

   Create a `.env` file in the `backend` directory and add the necessary environment variables. You can use the `.env.example` file as a reference.

   ```bash
    cp .env.example .env
   ```

   Then, edit the `.env` file to configure your environment variables.

5. **Create pgpass File**

   Create a `pgpass` file in your home directory to store your PostgreSQL credentials securely. The format is:

   ```bash
    hostname:port:database:username:password
   ```

   For example:

   ```bash
   localhost:5432:iot_hydroponik:admin_iot_db:your_password
   ```

   Make sure to set the correct permissions for the `pgpass` file:

   ```bash
    chmod 600 ~/.pgpass
   ```

   for windows users, you can create a file named `pgpass.conf` in the `%APPDATA%\postgresql\` directory with the same content. Reference: [Neon PostgreSQL Password](https://neon.com/postgresql/postgresql-administration/postgresql-password-file-pgpass/)

6. **Install TimescaleDB for PostgreSQL**

   This project uses TimescaleDB for time-series data. Follow the installation instructions for your operating system from the [TimescaleDB documentation](https://docs.tigerdata.com/self-hosted/latest/install/).

7. **Run Database Migrations**

   ```bash
    npm run migrate up
   ```

   or

    ```bash
    npm run migrate:latest
    ```

8. **Start the Backend Service**

   ```bash
    npm run start
   ```

## For Simulation

1. **Change to test directory**

   ```bash
    cd smart-hydroponic/backend/test
   ```

2. **Run the Simulation**

   ```bash
    py <nama_file_simulasi>.py
    ```

    File:
    - test_esp32.py
    - test_esp32_environment.py
    - test_esp8266_environment.py
