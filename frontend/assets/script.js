// Configuration
const config = {
    wsUrl: "ws://103.147.92.179:15000/device",
    controlUrl: "ws://103.147.92.179:15000/control",
    deviceId: "dashboard-device",
    actuatorDeviceId: "esp8266-actuator-device"
};

// DOM Elements
const elements = {
    pumpButton: document.getElementById("togglePumpButton"),
    lampButton: document.getElementById("toggleLampButton"),
    otomationButton: document.getElementById("toggleAutomation"),
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
    avg_temperature: document.getElementById("avg_temperature")
};

// State management
const state = {
    device_id: config.actuatorDeviceId,
    type: "command",
    data: {
        automationStatus: 0,
        pumpStatus: 0,
        lightStatus: 0
    }
};

// WebSocket connections
let wsDashboard = null;
let wsControl = null;

/**
 * Updates the UI based on current state
 */
function updateUI() {
    elements.statusPump.textContent = `Pump Status: ${state.data.pumpStatus}`;
    elements.statusLamp.textContent = `Lamp Status: ${state.data.lightStatus}`;
    
    // Update button states
    elements.pumpButton.disabled = state.data.automationStatus === 1;
    elements.lampButton.disabled = state.data.automationStatus === 1;
    
    // Update checkbox states without triggering events
    elements.otomationButton.checked = state.data.automationStatus === 1;
    elements.pumpButton.checked = state.data.pumpStatus === 1;
    elements.lampButton.checked = state.data.lightStatus === 1;
}

/**
 * Handles sensor data updates
 * @param {Object} data - The sensor data
 */
function handleSensorData(data) {
    elements.flowRate.textContent = data.flowRate;
    elements.distanceCm.textContent = data.distanceCm;
    elements.totalLitres.textContent = data.totalLitres;
    
    // Update moisture readings
    elements.moisture.forEach((element, i) => {
        element.textContent = data[`moisture${i + 1}`];
    });
    
    elements.moistureAvg.textContent = data.moistureAvg;
    elements.temperature_atas.textContent = data.temperatureAtas;
    elements.temperature_bawah.textContent = data.temperatureBawah;
    elements.humidity_atas.textContent = data.humidityAtas;
    elements.humidity_bawah.textContent = data.humidityBawah;
    elements.avg_temperature.textContent = ((data.temperatureAtas + data.temperatureBawah) / 2).toFixed(1);
    
    // Update state
    state.data.pumpStatus = data.pumpStatus ?? 0;
    state.data.lightStatus = data.lightStatus ?? 0;
    state.data.automationStatus = data.automationStatus ?? 0;
    
    updateUI();
}

/**
 * Connect to WebSocket services
 */
function connectWebSockets() {
    // Dashboard WebSocket
    wsDashboard = new WebSocket(config.wsUrl);
    
    wsDashboard.onopen = () => {
        console.log(`Connected to WebSocket at ${config.wsUrl}`);
        const registerData = {
            device_id: config.deviceId,
            type: "register"
        };
        wsDashboard.send(JSON.stringify(registerData));
        console.log("Register data sent:", registerData);
    };
    
    wsDashboard.onmessage = (event) => {
        console.log("Message received:", event.data);
        const data = JSON.parse(event.data);
        if (data.type === "update_data") {
            handleSensorData(data.data);
        }
    };
    
    wsDashboard.onclose = () => {
        console.log(`WebSocket connection to ${config.wsUrl} closed, attempting to reconnect...`);
        setTimeout(connectWebSockets, 5000);
    };
    
    wsDashboard.onerror = (error) => {
        console.error("WebSocket error:", error);
        wsDashboard.close();
    };
    
    // Control WebSocket
    wsControl = new WebSocket(config.controlUrl);
    
    wsControl.onopen = () => {
        console.log(`Connected to control WebSocket at ${config.controlUrl}`);
    };
    
    wsControl.onclose = () => {
        console.log(`Control WebSocket connection closed, attempting to reconnect...`);
        setTimeout(connectWebSockets, 5000);
    };
    
    wsControl.onerror = (error) => {
        console.error("Control WebSocket error:", error);
        wsControl.close();
    };
}

/**
 * Send command to control WebSocket
 */
function sendCommand() {
    if (wsControl && wsControl.readyState === WebSocket.OPEN) {
        wsControl.send(JSON.stringify(state));
        console.log("Command sent:", state);
    } else {
        console.error("Control WebSocket not connected!");
    }
}

// Event listeners
elements.otomationButton.addEventListener('change', () => {
    state.data.automationStatus = elements.otomationButton.checked ? 1 : 0;
    
    // Reset actuators when switching to automation
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
connectWebSockets();
updateUI();
