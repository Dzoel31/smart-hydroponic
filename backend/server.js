const WebSocket = require('ws');
const server = require('http');
const express = require('express');
const dotenv = require('dotenv');
const fs = require('fs');

const router_sensor = require('./routes/sensors');
const router_actuator = require('./routes/aktuator');
const db = require('./config/db');
const sql = fs.readFileSync('./database/iot_smart_hydroponic.sql', 'utf8');

db.connect((err, message) => {
    if (err) {
        console.log('Error connecting to database:', err.message);
        return;
    }
    console.log('Database connected');

    db.query(sql, (err, result) => {
        if (err) {
            console.log('Error executing SQL:', err.message);
            return;
        }
        console.log("Initialized database completed");
    });
});

dotenv.config();

const app = express();
const httpServer = server.createServer();
const wssMap = {
    '/plantdata': new WebSocket.Server({ noServer: true }),
    '/environmentdata': new WebSocket.Server({ noServer: true }),
    '/actuator': new WebSocket.Server({ noServer: true }),
    '/webcommand' : new WebSocket.Server({ noServer: true }),
};

Object.entries(wssMap).forEach(([path, wss]) => {
    wss.on('connection', (ws, request) => {
        console.log(`Client connected to ${path}`);

        ws.on('message', (message) => {
            console.log(`Received message from ${path}: ${message.toString()}`);
            wss.clients.forEach((client) => {
                if (client !== ws && client.readyState === WebSocket.OPEN) {
                    client.send(message);
                }
            });

            ws.send(`Server received: ${message.toString()}`);
        });

        ws.on('close', () => {
            console.log(`WebSocket disconnected from ${path}`);
        });

        ws.on('error', (err) => {
            console.error(`WebSocket error in ${path}:`, err);
        });
    });
});

httpServer.on('upgrade', (request, socket, head) => {
    const { pathname } = new URL(request.url, `wss://${request.headers.host}`);

    if (wssMap[pathname]) {
        wssMap[pathname].handleUpgrade(request, socket, head, (ws) => {
            wssMap[pathname].emit('connection', ws, request);
        });
    }
    else {
        socket.destroy();
    }
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