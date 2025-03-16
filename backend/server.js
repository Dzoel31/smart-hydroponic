const WebSocket = require('ws');
const http = require('http');
const express = require('express');
const dotenv = require('dotenv');
const fs = require('fs');

const router_sensor = require('./routes/sensor_route');
const router_actuator = require('./routes/actuator_route');
const db = require('./config/db');
const sql = fs.readFileSync('./database/iot_smart_hydroponic.sql', 'utf8');

db.connect((err) => {
    if (err) {
        console.log('Error connecting to database:', err.message);
        return;
    }
    console.log('Database connected');

    const checkTableQuery = `
        SELECT EXISTS (
            SELECT FROM information_schema.tables 
            WHERE table_schema = 'public' 
            AND table_name = 'sensor_data'
        );
    `;

    db.query(checkTableQuery, (err, result) => {
        if (err) {
            console.log('Error checking table existence:', err.message);
            return;
        }
        if (result.rows[0].exists) {
            console.log('Tables already exist, skipping initialization');
        } else {
            db.query(sql, (err) => {
                if (err) {
                    console.log('Error executing SQL:', err.message);
                    return;
                }
                console.log("Initialized database completed");
            });
        }
    });
});

dotenv.config();

const app = express();
const httpServer = http.createServer();

const wss = new WebSocket.Server({ noServer: true });

const clients = {
    data_moisture: [],
    data_environment: [],
    actuator: [],
    mois_temp: [],
};

wss.on('connection', (ws, request) => {
    const path = request.url;

    switch (path) {
        case '/plantdata':
            clients.data_moisture.push(ws);
            console.log('ESP32 (1) connected to /plantdata');
            break;
        case '/environmentdata':
            clients.data_environment.push(ws);
            console.log('ESP32 (2) connected to /environmentdata');
            break;
        case '/actuator':
            clients.actuator.push(ws);
            console.log('ESP8266 connected to /actuator');
            break;
        case '/mois_temp':
            clients.mois_temp.push(ws);
            console.log('Web connected to /mois_temp');
            break;
        default:
            console.log(`Unknown path: ${path}`);
            ws.close();
            break;
    }

    ws.on('message', (message) => {
        console.log(`Received on ${path}: ${message}`);
        let data;
        try {
            data = JSON.parse(message);
        } catch (e) {
            console.error(`Invalid JSON received on ${path}:`, e.message);
            ws.send('Invalid JSON format');
            return;
        }

        switch (path) {
            case '/plantdata':
                clients.data_moisture.forEach(client => client.send(JSON.stringify(data)));
                clients.mois_temp.forEach(client => client.send(JSON.stringify(data)));
                const plantQuery = 'INSERT INTO public.sensor_data (moisture1, moisture2, moisture3, moisture4, moisture5, moisture6, moistureavg, flowrate, totallitres, distancecm) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10) RETURNING *';
                const plantValues = [
                    data.moisture1,
                    data.moisture2,
                    data.moisture3,
                    data.moisture4,
                    data.moisture5,
                    data.moisture6,
                    data.moistureAvg,
                    data.flowRate,
                    data.totalLitres,
                    data.distanceCm
                ];
                db.query(plantQuery, plantValues, (err, result) => {
                    if (err) {
                        console.log('Error executing SQL:', err.message);
                        return;
                    }
                    console.log('Data inserted:', result.rows);
                });
                break;
            case '/environmentdata':
                clients.data_environment.forEach(client => client.send(JSON.stringify(data)));
                clients.mois_temp.forEach(client => client.send(JSON.stringify(data)));
                const envQuery = 'INSERT INTO public.environment_data (temperature_atas, humidity_atas, temperature_bawah, humidity_bawah, tds) VALUES ($1, $2, $3, $4, $5) RETURNING *';
                const envValues = [
                    data.temperature_atas,
                    data.humidity_atas,
                    data.temperature_bawah,
                    data.humidity_bawah,
                    data.tdsValue
                ];
                db.query(envQuery, envValues, (err, result) => {
                    if (err) {
                        console.log('Error executing SQL:', err.message);
                        return;
                    }
                    console.log('Data inserted:', result.rows);
                });
                break;
            case '/actuator':
                clients.actuator.forEach(client => client.send(JSON.stringify(data)));
                const actuatorQuery = 'INSERT INTO public.actuator_data (pumpstatus, lightstatus, automationstatus) VALUES ($1, $2, $3) RETURNING *';
                const actuatorValues = [data.pumpStatus, data.lightStatus, data.otomationStatus];
                db.query(actuatorQuery, actuatorValues, (err, result) => {
                    if (err) {
                        console.log('Error executing SQL:', err.message);
                        return;
                    }
                    console.log('Data inserted:', result.rows);
                });
                break;
            case '/mois_temp':
                clients.mois_temp.forEach(client => client.send(JSON.stringify(data)));
                break;
            default:
                console.log(`Unknown path: ${path}`);
                break;
        }
    });

    const interval = setInterval(() => {
        if (ws.readyState === WebSocket.OPEN) {
            ws.ping();
        }
    }, 10000);

    ws.on('pong', () => {
        console.log(`Pong received from ${path}`);
    });

    ws.on('close', () => {
        console.log(`Client disconnected from ${path}`);
    });
});

httpServer.on('upgrade', (request, socket, head) => {
    wss.handleUpgrade(request, socket, head, (ws) => {
        wss.emit('connection', ws, request);
    });
});

app.use(express.json());
app.use(router_sensor);
app.use(router_actuator);
app.listen(process.env.PORT, process.env.IP4_ADDRESS, () => {
    console.log(`Server is running on http://${process.env.IP4_ADDRESS}:${process.env.PORT}`);
});
httpServer.listen(process.env.PORT_WS, process.env.IP4_ADDRESS, () => {
    console.log(`WebSocket server is running on ws://${process.env.IP4_ADDRESS}:${process.env.PORT_WS}`);
});
