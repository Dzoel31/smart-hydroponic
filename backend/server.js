const WebSocket = require('ws');
const express = require('express');
const dotenv = require('dotenv');

const router = require('./routes/sensors');

const db = require('./config/db');

db.connect((err) => {
    if (err) {
        console.log('Error connecting to database: ', err.message);
        return;
    }

    console.log('Connected to database');
});

dotenv.config();

const app = express();
const wss = new WebSocket.Server({
    host: process.env.IP4_ADDRESS_DEV,
    port: process.env.PORT_WS
});

wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(message) {
        console.log('received: %s', message.toString());

        try {
            const { temperature, humidity } = JSON.parse(message);

            const query = `INSERT INTO sensor_data (temperature, humidity) VALUES (${temperature}, ${humidity})`;
            db.query(query, [temperature, humidity], (err, result) => {
                if (err) {
                    console.log('Error inserting data: ', err);
                    return;
                }

                console.log('Data inserted');
            });
        } catch (error) {
            console.log('Got an error', error);
        }
    });

    ws.on('close', function close() {
        console.log('ESP32 disconnected');
    });
});

app.use(express.json());
app.use(router);

app.listen(process.env.PORT, process.env.IP4_ADDRESS_DEV, () => {
    console.log(`Server is running on : ${process.env.IP4_ADDRESS_DEV}:${process.env.PORT}`);
});