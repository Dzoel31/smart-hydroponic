const WebSocket = require('ws');
const http = require('http');
const express = require('express');
const cors  = require('cors');
const dotenv = require('dotenv');
const fs = require('fs');

const router_sensor = require('./routes/sensor_route');
const router_actuator = require('./routes/actuator_route');
const { updateCache, getAlldata, clearCache, isDataComplete } = require('./utils/data_cache');
const { plantDataUpdate, environmentDataUpdate, actuatorDataUpdate } = require('./utils/insert_data');
const db = require('./config/db');

db.connect((err) => {
    if (err) {
        console.log('Error connecting to database:', err.message);
        return;
    }
    console.log('Database connected');
});

dotenv.config();

const app = express();
const server = http.createServer(app);

const heartbeat = () => {
    this.isAlive = true;
    console.log("Heartbeat received from device");
}

const wssDevice = new WebSocket.Server({ noServer: true, path: '/ws/smart-hydroponic/device' });
const wssControl = new WebSocket.Server({ noServer: true, path: '/ws/smart-hydroponic/control' });

const deviceClients = new Map();
const dashboardClients = new Set();

server.on('upgrade', (request, socket, head) => {
    if (request.url === '/ws/smart-hydroponic/device') {
        wssDevice.handleUpgrade(request, socket, head, (ws) => {
            wssDevice.emit('connection', ws, request);
        });
    } else if (request.url === '/ws/smart-hydroponic/control') {
        wssControl.handleUpgrade(request, socket, head, (ws) => {
            wssControl.emit('connection', ws, request);
        });
    } else {
        socket.destroy(); // Close the socket if the URL doesn't match
    }
});

wssDevice.on('connection', (ws) => {
    ws.isAlive = true;
    console.log('Device connected');

    ws.on("message", (message) => {
        let payload;
        try {
            payload = JSON.parse(message);
            console.log("[DEVICE WS] Data received from device:", payload);
        } catch (e) {
            console.error("Error parsing message:", e.message);
            return;
        }
        
        const { device_id, type, data } = payload;

        if (type === 'register') {
            deviceClients.set(device_id, ws);
            console.log(`[DEVICE WS] Registered device ${device_id}`);
            if (device_id === "dashboard-device" || device_id === "esp8266-actuator-device") {
                dashboardClients.add(ws);
                console.log(`[DEVICE WS] Registered dashboard device ${device_id}`);
            }
            ws.send(JSON.stringify({ type: 'register', status: 'success' }));
        } else if (type === 'update_data') {
            // Process data based on device type
            if (device_id === "esp32-plant-device") {
                const moistureValues = [
                    data.moisture1,
                    data.moisture2,
                    data.moisture3,
                    data.moisture4,
                    data.moisture5,
                    data.moisture6,
                ].filter(value => value > 0);

                if (moistureValues.length > 0) {
                    const averageMoisture = moistureValues.reduce((a, b) => a + b, 0) / moistureValues.length;
                    data.moistureAvg = parseFloat(averageMoisture.toPrecision(3));
                } else {
                    console.warn("No valid moisture values found for device:", device_id);
                }
            } else if (device_id === "esp32-environment-device") {
                data.temperatureAtas = parseFloat(data.temperatureAtas.toPrecision(3));
                data.temperatureBawah = parseFloat(data.temperatureBawah.toPrecision(3));
                data.temperatureAvg = parseFloat(((data.temperatureAtas + data.temperatureBawah) / 2).toPrecision(3));
                data.humidityAtas = parseFloat(data.humidityAtas.toPrecision(3));
                data.humidityBawah = parseFloat(data.humidityBawah.toPrecision(3));
                data.humidityAvg = parseFloat(((data.humidityAtas + data.humidityBawah) / 2).toPrecision(3));
            } else if (device_id === "esp8266-actuator-device") {
                data.pumpStatus = parseInt(data.pumpStatus) || 0;
                data.lightStatus = parseInt(data.lightStatus) || 0;
                data.automationStatus = parseInt(data.automationStatus) || 0;
            }

            updateCache(device_id, data);
            
            if (isDataComplete(getAlldata())) {
                const allData = getAlldata();
                const completePayload = {
                    ...allData["esp32-plant-device"],
                    ...allData["esp32-environment-device"],
                    ...allData["esp8266-actuator-device"],
                };
                
                console.log("Data complete, broadcasting to all dashboards:", completePayload);
                
                // Send to all dashboard clients
                for (const client of dashboardClients) {      
                    if (client.readyState === WebSocket.OPEN) {                        
                        client.send(JSON.stringify({ type: 'update_data', data: completePayload }));
                    }
                }
                
                // Insert data into database
                plantDataUpdate("esp32-plant-device", allData["esp32-plant-device"]);
                environmentDataUpdate("esp32-environment-device", allData["esp32-environment-device"]);
                actuatorDataUpdate("esp8266-actuator-device", allData["esp8266-actuator-device"]);
                
                clearCache(); // Clear cache after broadcasting and storing
            } else {
                console.log("Data not complete, not broadcasting to devices");
            }
        }
    });

    const pingInterval = setInterval(function ping() {
        wssDevice.clients.forEach(function each(ws) {
            if (ws.isAlive === false) return ws.terminate();
            ws.isAlive = false;
            ws.ping(() => {
                console.log("Ping");
            });
        })
    }, 30000); // Send a ping every 30 seconds

    ws.on("pong", heartbeat);

    ws.on('close', () => {
        for (const [device_id, client] of deviceClients.entries()) {
            if (client === ws) {
                deviceClients.delete(device_id);
                console.log(`[DEVICE WS] Device ${device_id} disconnected`);
                break;
            }
        }
        clearInterval(pingInterval);
    });
});

wssControl.on('connection', (ws, req) => {
    console.log('Dashboard connected');
    dashboardClients.add(ws);

    ws.on('message', (message) => {
        let payload;
        try {
            payload = JSON.parse(message);
            console.log("Data received from dashboard:", payload);
        } catch (e) {
            console.error("Error parsing message:", e.message);
            return;
        }

        const { device_id, type, data } = payload;

        const targetDevice = deviceClients.get(device_id);
        if (targetDevice) {
            targetDevice.send(JSON.stringify({ type: 'command', data }));
            console.log(`[CONTROL] Command sent to device ${device_id}:`, data);
        } else {
            console.warn(`[CONTROL] Device ${device_id} not found`);
        }
    });

    const pingInterval = setInterval(function ping() {
        wssControl.clients.forEach(function each(ws) {
            if (ws.isAlive === false) return ws.terminate();
            ws.isAlive = false;
            ws.ping(() => {
                console.log("Ping");
            });
        })
    }, 30000); // Send a ping every 30 seconds

    ws.on("pong", heartbeat);
    
    ws.on('close', () => {
        dashboardClients.delete(ws);
        console.log('Dashboard disconnected');
        clearInterval(pingInterval);
    });
});

app.use(cors())
app.use(express.json());
app.use(router_sensor);
app.use(router_actuator);

server.listen(process.env.PORT, process.env.HOST, () => {
    console.log(`Server is running on port ws://localhost:${process.env.PORT}/ws/smart-hydroponic/device`);
    console.log(`Server is running on port ws://localhost:${process.env.PORT}/ws/smart-hydroponic/control`);
    console.log(`Server is running on port http://localhost:${process.env.PORT}`);
});
