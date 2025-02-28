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
            let parsedMessage;
            try {
            parsedMessage = JSON.parse(message);
            } catch (e) {
            console.error(`Invalid JSON received from ${path}:`, e.message);
            ws.send('Invalid JSON format');
            return;
            }

            wss.clients.forEach((client) => {
            if (client !== ws && client.readyState === WebSocket.OPEN) {
                client.send(JSON.stringify(parsedMessage));
            }
            });

            if (path === '/plantdata') {
                const query = 'INSERT INTO plant_data (column1, column2, column3, column4, column5, column6, column7, column8, column9, column10) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10) RETURNING *';
                const values = [parsedMessage.column1, parsedMessage.column2, parsedMessage.column3, parsedMessage.column4, parsedMessage.column5, parsedMessage.column6, parsedMessage.column7, parsedMessage.column8, parsedMessage.column9, parsedMessage.column10];

                db.query(query, values, (err, result) => {
                    if (err) {
                        console.log('Error executing SQL:', err.message);
                        return;
                    }
                    console.log('Data inserted:', result.rows);
                })
            }

            if (path === '/environmentdata') {
                const query = 'INSERT INTO environment_data (column1, column2, column3, column4, column5) VALUES ($1, $2, $3, $4, $5) RETURNING *';
                const values = [parsedMessage.column1, parsedMessage.column2, parsedMessage.column3, parsedMessage.column4, parsedMessage.column5];

                db.query(query, values, (err, result) => {
                    if (err) {
                        console.log('Error executing SQL:', err.message);
                        return;
                    }
                    console.log('Data inserted:', result.rows);
                })
            }

            if (path === '/actuator') {
                const query = 'INSERT INTO actuator (column1, column2) VALUES ($1, $2) RETURNING *';
                const values = [parsedMessage.column1, parsedMessage.column2];

                db.query(query, values, (err, result) => {
                    if (err) {
                        console.log('Error executing SQL:', err.message);
                        return;
                    }
                    console.log('Data inserted:', result.rows);
                })
            }

            ws.send(`Server received: ${JSON.stringify(parsedMessage)}`);
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