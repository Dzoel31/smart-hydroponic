const express = require('express');
const cors = require('cors');
const dotenv = require('dotenv');
const http = require('http');
const WebSocket = require('ws');

const router_sensor = require('./routes/sensor_route');
const router_actuator = require('./routes/actuator_route');
const router_environment = require('./routes/environment_route');
const router_auth = require('./routes/auth_route');

const db = require('./config/db');

const { deviceLogger, errorLogger } = require("./utils/logger");
const { plantDataUpdate, environmentDataUpdate, actuatorDataUpdate } = require('./utils/insert_data');

db.connect((err) => {
    if (err) {
        console.log('Error connecting to database:', err.message);
        errorLogger.error('Error connecting to database:', err.message);
        return;
    }
    deviceLogger.info('Database connected');
});

dotenv.config();

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

app.use(cors());
app.use(express.json());
app.get('/', (req, res) => {
    res.send('Welcome to Smart Hydroponic API');
});

app.use(router_sensor);
app.use(router_actuator);
app.use(router_environment);
app.use('/', router_auth);

const rooms = {
    'plant': new Set(),
    'environment': new Set(),
    'command': new Set(),
}

wss.on('connection', (ws) => {
    ws.room = null; // Initialize room for the client

    ws.on('message', (message) => {
        try {
            const data = JSON.parse(message);

            if (data.type === "join") {
                ws.room = data.room; // Set the room for the client
                // Add client to the specific room set
                if (rooms[data.room]) {
                    rooms[data.room].add(ws);
                } else {
                    deviceLogger.warn(`Room ${data.room} does not exist`);
                }
            }

            if (data.type === "update_data") {
                deviceLogger.info(`Received update data: ${data.data}`);
                const targetRoom = data.room;
                const broadcast = data.broadcast || "command";

                if (targetRoom === "plant") {
                    plantDataUpdate(data.deviceId, data.data)
                        .then()
                        .catch(err => {
                            errorLogger.error('Error updating plant data:', err);
                        });
                }

                if (targetRoom === "environment") {
                    environmentDataUpdate(data.deviceId, data.data)
                        .then()
                        .catch(err => {
                            errorLogger.error('Error updating environment data:', err);
                        });
                }
                if (targetRoom === "command") {
                    actuatorDataUpdate(data.deviceId, data.data)
                        .then()
                        .catch(err => {
                            errorLogger.error('Error updating actuator data:', err);
                        });
                }
            }

            if (data.type === "command" || data.broadcast === "command") {
                const targetRoom = data.room;
                // Broadcast the message to all connected clients
                // handle for moisture data

                if (data.room === "plant") {
                    const moistureKeys = Object.keys(data).filter(key => /^moisture\d+$/.test(key));
                    const moistureValues = moistureKeys.map(key => Number(data[key])).filter(val => !isNaN(val));
                    const moistureAvg = moistureValues.length > 0 ? moistureValues.reduce((sum, val) => sum + val) / moistureValues.length : 0;
                    data.data.moistureAvg = moistureAvg;
                }
                if (data.room === "environment") {
                    const avgTemperature = (data.data.temperatureAtas + data.data.temperatureBawah) / 2;
                    data.data.temperatureAvg = avgTemperature;
                }

                rooms[targetRoom].forEach(client => {
                    if (client.readyState === WebSocket.OPEN) {
                        client.send(JSON.stringify({
                            type: "command",
                            data: data.data
                        }));
                    }
                });
            }
        } catch (error) {
            deviceLogger.error('Error processing message:', error);
        }
    });

    ws.on('close', () => {
        if (ws.room && rooms[ws.room]) {
            rooms[ws.room].delete(ws); // Remove client from the room
            deviceLogger.info(`Client disconnected from room: ${ws.room}`);
        }
        deviceLogger.info('Client disconnected');
    });
});


server.listen(process.env.PORT, process.env.HOST, () => {
    deviceLogger.info(`Server is running on port http://localhost:${process.env.PORT}`);
});
