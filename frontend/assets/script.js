/**
 * Utility to handle fetch requests with error handling and optional JSON parsing
 */
async function apiRequest(url, options = {}, parseJson = true) {
    try {
        url = `http://172.23.14.161:15000${url}`;
        const response = await fetch(url, options);
        if (!response.ok) {
            throw new Error(`Request failed: ${response.statusText}`);
        }
        return parseJson ? await response.json() : response;
    } catch (error) {
        console.error(`API request error (${url}):`, error);
        throw error;
    }
}

// API Endpoints
const endpoints = {
    sensorLatest: '/api/smart-hydroponic/v1/sensor/latest',
    environmentLatest: '/api/smart-hydroponic/v1/environment/latest',
};

// DOM Elements
const elements = {
    pumpButton: document.getElementById("togglePumpButton"),
    lampButton: document.getElementById("toggleLampButton"),
    automationButton: document.getElementById("toggleAutomation"),
    statusPump: document.getElementById("statusPump"),
    statusLamp: document.getElementById("statusLamp"),
    flowRate: document.getElementById("flowRate"),
    distanceCm: document.getElementById("distanceCm"),
    totalLitres: document.getElementById("totalLitres"),
    moisture: Array.from({ length: 6 }, (_, i) => document.getElementById(`moisture${i + 1}`)),
    moistureAvg: document.getElementById("moistureAvg"),
    temperature_atas: document.getElementById("temperature_atas"),
    temperature_bawah: document.getElementById("temperature_bawah"),
    humidity_atas: document.getElementById("humidity_atas"),
    humidity_bawah: document.getElementById("humidity_bawah"),
    avg_temperature: document.getElementById("avg_temperature"),
};

// State management
const state = {
    deviceId: "dashboard-device",
    type: "command",
    room: "command",
    data: {
        automationStatus: 0,
        pumpStatus: 0,
        lightStatus: 0
    }
};

/**
 * Updates the UI based on current state
 */
function updateUI() {
    elements.statusPump.textContent = `Pump Status: ${state.data.pumpStatus}`;
    elements.statusLamp.textContent = `Lamp Status: ${state.data.lightStatus}`;
    elements.pumpButton.disabled = elements.lampButton.disabled = state.data.automationStatus === 1;
    elements.automationButton.checked = state.data.automationStatus === 1;
    elements.pumpButton.checked = state.data.pumpStatus === 1;
    elements.lampButton.checked = state.data.lightStatus === 1;
}

/**
 * Handles sensor data updates and updates state/UI
 */

// Handler untuk data sensor
function handleSensorData(data) {
    elements.flowRate.textContent = data[0].flowrate ?? '-';
    elements.distanceCm.textContent = data[0].distance_cm ?? '-';
    elements.totalLitres.textContent = data[0].total_litres ?? '-';
    elements.moisture.forEach((el, i) => { el.textContent = data[0][`moisture${i + 1}`] ?? '-'; });
    elements.moistureAvg.textContent = data[0].moistureavg !== undefined && data[0].moistureavg !== null
        ? Number(data[0].moistureavg).toFixed(2)
        : '-';
}

// Handler untuk data environment
function handleEnvironmentData(data) {
    elements.temperature_atas.textContent = data[0].temperature_atas ?? '-';
    elements.temperature_bawah.textContent = data[0].temperature_bawah ?? '-';
    elements.humidity_atas.textContent = data[0].humidity_atas ?? '-';
    elements.humidity_bawah.textContent = data[0].humidity_bawah ?? '-';
    elements.avg_temperature.textContent = ((data[0].temperature_atas ?? 0) + (data[0].temperature_bawah ?? 0)) / 2;
}

// Handler untuk data actuator
function handleActuatorData(data) {
    state.data.pumpStatus = data[0].pump_status ?? 0;
    state.data.lightStatus = data[0].light_status ?? 0;
    state.data.automationStatus = data[0].automation_status ?? 0;
    updateUI();
}

/**
 * Fetch and handle sensor data from API
 */
async function fetchSensorData() {
    const dataToFetch = [endpoints.sensorLatest, endpoints.environmentLatest];
    try {
        for (const endpoint of dataToFetch) {
            const data = await apiRequest(endpoint);
            if (endpoint === endpoints.sensorLatest) {
                handleSensorData(data.data);
            } else if (endpoint === endpoints.environmentLatest) {
                handleEnvironmentData(data.data);
            }
        }
    } catch (e) {
        console.error("Error fetching sensor data:", e);
    }
}

/**
 * Send current state as command to API
 */

let ws;

function registerWebSocket() {
    const registerData = {
        deviceId: state.deviceId,
        type: "join",
        room: state.room
    };
    ws = new WebSocket("ws://172.23.14.161:15000");

    ws.onopen = () => {
        ws.send(JSON.stringify(registerData));
        console.log("WebSocket connection established and registered.");
    };
    ws.onmessage = (event) => {
        const message = JSON.parse(event.data);
        state.data.automationStatus = message.data.automationStatus;
        state.data.pumpStatus = message.data.pumpStatus;
        state.data.lightStatus = message.data.lightStatus;
        updateUI();
    };
}

async function sendCommand() {
    try {
        if (ws && ws.readyState === WebSocket.OPEN) {
            const commandData = {
                deviceId: state.deviceId,
                type: state.type,
                room: state.room,
                data: {
                    automationStatus: state.data.automationStatus,
                    pumpStatus: state.data.pumpStatus,
                    lightStatus: state.data.lightStatus
                }
            };
            ws.send(JSON.stringify(commandData));
        }
    } catch (error) {
        console.error("Error sending command:", error);
    }
}

// Event listeners
elements.automationButton.addEventListener('change', () => {
    state.data.automationStatus = elements.automationButton.checked ? 1 : 0;
    if (state.data.automationStatus === 1) {
        state.data.pumpStatus = 0;
        state.data.lightStatus = 0;
    }
    updateUI();
    sendCommand();
});
elements.pumpButton.addEventListener("change", () => {
    state.data.pumpStatus = elements.pumpButton.checked ? 1 : 0;
    updateUI();
    sendCommand();
});
elements.lampButton.addEventListener("change", () => {
    state.data.lightStatus = elements.lampButton.checked ? 1 : 0;
    updateUI();
    sendCommand();
});

// Initialize
fetchSensorData();
registerWebSocket();
updateUI();
setInterval(fetchSensorData, 5000);
