const WebSocket = require('ws');
const express = require('express');
const dotenv = require('dotenv');
const fs = require('fs');

const router_sensor = require('./routes/sensors');
const router_actuator = require('./routes/aktuator');
const db = require('./config/db');
const sql = fs.readFileSync('./database/iot_smart_hydroponic.sql', 'utf8');

db.connect((err) => {
    if (err) {
        console.log('Error connecting to database: ', err.message);
        return;
    }

    console.log('Connected to database');
});

db.query(sql, (err, result) => {
    if (err) {
        console.log('Error creating database: ', err.message);
        return;
    }
    console.log("SQL Database created");
});

dotenv.config();

const app = express();
const wss = new WebSocket.Server({
    host: process.env.IP4_ADDRESS,
    port: process.env.PORT_WS
});

const allowedSensorType = [
    'plant_ESP32',
    'environment_ESP32'
]

const allowedActuatorType = [
    "pump_light_ESP8266",
]

let clients = new Set();
var combinedData = {};

wss.on('connection', function connection(ws) {
    clients.add(ws);
    ws.on('message', function incoming(message) {
        console.log('received: %s', message.toString());
        
        try {
            const data = JSON.parse(message.toString());
            if (allowedSensorType.includes(data.type)) {
                const type = data.type;
                delete data.type;

                if (!global.storeData) {
                    global.storeData = {};
                }

                global.storeData[type] = { ...data };

                combinedData = {
                    plantData: {
                        moisture1: global.storeData['plant_ESP32']?.moisture1 || 0,
                        moisture2: global.storeData['plant_ESP32']?.moisture2 || 0,
                        moisture3: global.storeData['plant_ESP32']?.moisture3 || 0,
                        moisture4: global.storeData['plant_ESP32']?.moisture4 || 0,
                        moisture5: global.storeData['plant_ESP32']?.moisture5 || 0,
                        moisture6: global.storeData['plant_ESP32']?.moisture6 || 0,
                        moistureAvg: global.storeData['plant_ESP32']?.moistureAvg || 0,
                        flowRate: global.storeData['plant_ESP32']?.flowRate || 0,
                        totalLitres: global.storeData['plant_ESP32']?.totalLitres || 0,
                        distanceCm: global.storeData['plant_ESP32']?.distanceCm || 0
                    },
                    environmentData: {
                        temperature_atas: global.storeData['environment_ESP32']?.temperature_atas || 0,
                        humidity_atas: global.storeData['environment_ESP32']?.humidity_atas || 0,
                        temperature_bawah: global.storeData['environment_ESP32']?.temperature_bawah || 0,
                        humidity_bawah: global.storeData['environment_ESP32']?.humidity_bawah || 0,
                        tds: global.storeData['environment_ESP32']?.tds || 0
                    }
                };
                

                query = `INSERT INTO sensor_data (
                    moisture1,
                    moisture2,
                    moisture3,
                    moisture4,
                    moisture5,
                    moisture6,
                    moistureAvg,
                    water_flowrate,
                    total_litres,
                    distanceCm,
                    temperature_atas,
                    humidity_atas,
                    temperature_bawah,
                    humidity_bawah,
                    tds) VALUES (
                        ${combinedData.plantData.moisture1},
                        ${combinedData.plantData.moisture2},
                        ${combinedData.plantData.moisture3},
                        ${combinedData.plantData.moisture4},
                        ${combinedData.plantData.moisture5},
                        ${combinedData.plantData.moisture6},
                        ${combinedData.plantData.moistureAvg},
                        ${combinedData.plantData.flowRate},
                        ${combinedData.plantData.totalLitres},
                        ${combinedData.plantData.distanceCm},
                        ${combinedData.environmentData.temperature_atas},
                        ${combinedData.environmentData.humidity_atas},
                        ${combinedData.environmentData.temperature_bawah},
                        ${combinedData.environmentData.humidity_bawah},
                        ${combinedData.environmentData.tds}
                    )`;

                db.query(query, (err, result) => {
                    if (err) {
                        console.log('Error inserting data: ', err);
                        return;
                    }

                    console.log('Data inserted successfully');
                });
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
                        ${global.storeData["pump_light_ESP8266"].pumpStatus},
                        ${global.storeData["pump_light_ESP8266"].lightStatus}
                    )`;

                db.query(query, (err, result) => {
                    if (err) {
                        console.log('Error inserting data: ', err);
                        return;
                    }

                    console.log('Data inserted successfully');
                });
            }

            clients.forEach((client) => {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(JSON.stringify(combinedData));
                }
            });            
            
        } catch (error) {
            console.log('Got an error while parsing data:', error);
        }
    });

    ws.on('open', function open() {
        console.log('ESP32 and ESP8266 connected');
    });

    ws.on('close', function close() {
        console.log('ESP32 and ESP8266 disconnected');
        clients.delete(ws);
    });

    ws.on('error', function error(err) {
        console.log('WebSocket error: ', err);
    });
});

app.use(express.json());
app.use(router_sensor);
app.use(router_actuator);
app.listen(process.env.PORT, process.env.IP4_ADDRESS, () => {
    console.log(`Server is running on http://${process.env.IP4_ADDRESS}:${process.env.PORT}`);
});