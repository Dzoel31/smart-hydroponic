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
// const wssMap = {
//     '/plantdata': new WebSocket.Server({ noServer: true }),
//     '/environmentdata': new WebSocket.Server({ noServer: true }),
//     '/actuator': new WebSocket.Server({ noServer: true }),
//     '/webcommand' : new WebSocket.Server({ noServer: true }),
// };

plantdata_ws = new WebSocket.Server({ noServer: true });
environmentdata_ws = new WebSocket.Server({ noServer: true });
actuator_ws = new WebSocket.Server({ noServer: true });
webcommand_ws = new WebSocket.Server({ noServer: true });

let actuator_socket = null;
let plantdata_socket = null;
let environmentdata_socket = null;
let webcommand_socket = null;

const handlePlantData = (ws, request) => {
    console.log(`Client connected to /plantdata`);

    ws.on('message', (message) => {
        console.log(`Received message from /plantdata: ${message.toString()}`);
        let parsedMessage;
        try {
            parsedMessage = JSON.parse(message);
        } catch (e) {
            console.error(`Invalid JSON received from /plantdata:`, e.message);
            ws.send('Invalid JSON format');
            return;
        }

        if (actuator_socket && actuator_socket.readyState === WebSocket.OPEN) {
            actuator_socket.send(JSON.stringify(parsedMessage));
            console.log(`Sent message to /actuator: ${parsedMessage}`);
        } else {
            console.warn('Actuator Websocket is not connected');
        }

        const query = 'INSERT INTO public.sensor_data (moisture1, moisture2, moisture3, moisture4, moisture5, moisture6, moistureavg, flowrate, totallitres, distancecm) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10) RETURNING *';
        const values = [
            parsedMessage.moisture1, 
            parsedMessage.moisture2, 
            parsedMessage.moisture3, 
            parsedMessage.moisture4, 
            parsedMessage.moisture5, 
            parsedMessage.moisture6, 
            parsedMessage.moistureAvg, 
            parsedMessage.flowRate, 
            parsedMessage.totalLitres, 
            parsedMessage.distanceCm
        ];

        db.query(query, values, (err, result) => {
            if (err) {
                console.log('Error executing SQL:', err.message);
                return;
            }
            console.log('Data inserted:', result.rows);
        });

        ws.send(JSON.stringify(parsedMessage));
    });

    ws.on('close', () => {
        console.log(`WebSocket disconnected from /plantdata`);
    });

    ws.on('error', (err) => {
        console.error(`WebSocket error in /plantdata:`, err);
    });
};

const handleEnvironmentData = (ws, request) => {
    console.log(`Client connected to /environmentdata`);

    ws.on('message', (message) => {
        console.log(`Received message from /environmentdata: ${message.toString()}`);
        let parsedMessage;
        try {
            parsedMessage = JSON.parse(message);
        } catch (e) {
            console.error(`Invalid JSON received from /environmentdata:`, e.message);
            ws.send('Invalid JSON format');
            return;
        }

        if (actuator_socket && actuator_socket.readyState === WebSocket.OPEN) {
            actuator_socket.send(JSON.stringify(parsedMessage));
            console.log(`Sent message to /actuator: ${parsedMessage}`);
        } else {
            console.warn('Actuator Websocket is not connected');
        }

        const query = 'INSERT INTO public.environment_data (temperature_atas, humidity_atas, temperature_bawah, humidity, tds) VALUES ($1, $2, $3, $4, $5) RETURNING *';
        const values = [
            parsedMessage.temperature_atas, 
            parsedMessage.humidity_atas, 
            parsedMessage.temperature_bawah, 
            parsedMessage.humidity_bawah, 
            parsedMessage.tds
        ];

        db.query(query, values, (err, result) => {
            if (err) {
                console.log('Error executing SQL:', err.message);
                return;
            }
            console.log('Data inserted:', result.rows);
        });

        ws.send(JSON.stringify(parsedMessage));
    });

    ws.on('close', () => {
        console.log(`WebSocket disconnected from /environmentdata`);
    });

    ws.on('error', (err) => {
        console.error(`WebSocket error in /environmentdata:`, err);
    });
};

const handleActuator = (ws, request) => {
    console.log(`Client connected to /actuator`);

    actuator_socket = ws;

    ws.on('message', (message) => {
        console.log(`Received message from /actuator: ${message.toString()}`);
        let parsedMessage;
        try {
            parsedMessage = JSON.parse(message);
        } catch (e) {
            console.error(`Invalid JSON received from /actuator:`, e.message);
            ws.send('Invalid JSON format');
            return;
        }

        const query = 'INSERT INTO public.actuator_data (pumpstatus, lightstatus, automationstatus) VALUES ($1, $2, $3) RETURNING *';
        const values = [parsedMessage.pumpStatus, parsedMessage.lightStatus, parsedMessage.otomationStatus];

        db.query(query, values, (err, result) => {
            if (err) {
                console.log('Error executing SQL:', err.message);
                return;
            }
            console.log('Data inserted:', result.rows);
        });

        ws.send(JSON.stringify(parsedMessage));
    });

    ws.on('close', () => {
        console.log(`WebSocket disconnected from /actuator`);
        actuator_socket = null;
    });

    ws.on('error', (err) => {
        console.error(`WebSocket error in /actuator:`, err);
    });
};

const handleWebCommand = (ws, request) => {
    console.log(`Client connected to /webcommand`);

    ws.on('message', (message) => {
        console.log(`Received message from /webcommand: ${message.toString()}`);
        let parsedMessage;
        try {
            parsedMessage = JSON.parse(message);
        } catch (e) {
            console.error(`Invalid JSON received from /webcommand:`, e.message);
            ws.send('Invalid JSON format');
            return;
        }

        if (actuator_socket && actuator_socket.readyState === WebSocket.OPEN) {
            actuator_socket.send(JSON.stringify(parsedMessage));
            console.log(`Sent message to /actuator: ${parsedMessage}`);            
        } else {
            console.warn('Actuator Websocket is not connected');
        }

        ws.send(JSON.stringify(parsedMessage));
    });

    ws.on('close', () => {
        console.log(`WebSocket disconnected from /webcommand`);
    });

    ws.on('error', (err) => {
        console.error(`WebSocket error in /webcommand:`, err);
    });
};

plantdata_ws.on('connection', handlePlantData);
environmentdata_ws.on('connection', handleEnvironmentData);
actuator_ws.on('connection', handleActuator);
webcommand_ws.on('connection', handleWebCommand);


httpServer.on('upgrade', (request, socket, head) => {
    if (request.url === '/plantdata') {
        plantdata_ws.handleUpgrade(request, socket, head, (ws) => {
            plantdata_ws.emit('connection', ws, request);
        });
    }
    else if (request.url === '/environmentdata') {
        environmentdata_ws.handleUpgrade(request, socket, head, (ws) => {
            environmentdata_ws.emit('connection', ws, request);
        });
    }
    else if (request.url === '/actuator') {
        actuator_ws.handleUpgrade(request, socket, head, (ws) => {
            actuator_ws.emit('connection', ws, request);
        });
    }
    else if (request.url === '/webcommand') {
        webcommand_ws.handleUpgrade(request, socket, head, (ws) => {
            webcommand_ws.emit('connection', ws, request);
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