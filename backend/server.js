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

const allowedSensorType = [
    'plant_ESP32',
    'environment_ESP32'
]

wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(message) {
        console.log('received: %s', message.toString());
        
        try {
            const data = JSON.parse(message.toString());
            if (allowedSensorType.includes(data.type)) {
                // pop the type from the data
                const type = data.type;
                delete data.type;

                // store the data
                if (!global.storeData) {
                    global.storeData = {};
                }

                global.storeData[type] = { ...data };

                // Combine data from both sensor types into one object
                const combinedData = {
                    plantData: global.storeData['plant_ESP32'] || {},
                    environmentData: global.storeData['environment_ESP32'] || {}
                };

                console.log('Combined Data:', combinedData);
            }

            if (allowedActuatorType.includes(data.type)) {
                // pop the type from the data
                const type = data.type;
                delete data.type;

                // store the data
                if (!global.storeData) {
                    global.storeData = {};
                }

                global.storeData[type] = { ...data};

                query = `INSERT INTO actuator_data (
                    pumpStatus,
                    lightStatus) VALUES (
                        ${global.storeData.pumpStatus},
                        ${global.storeData.lightStatus}
                    )`;

                db.query(query, (err, result) => {
                    if (err) {
                        console.log('Error inserting data: ', err);
                        return;
                    }

                    console.log('Data inserted successfully');
                });
            }
            
        } catch (error) {
            console.log('Got an error while parsing data:', error);
        }
    });

    ws.on('close', function close() {
        console.log('ESP32 disconnected');
    });

    ws.on('error', function error(err) {
        console.log('WebSocket error: ', err);
    });
});

app.use(express.json());
app.use(router);

app.listen(process.env.PORT, process.env.IP4_ADDRESS_DEV, () => {
    console.log(`Server is running on http://${process.env.IP4_ADDRESS_DEV}:${process.env.PORT}`);
});