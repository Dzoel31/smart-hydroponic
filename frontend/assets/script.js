// Configuration
const config = {
    wsUrl: "ws://103.147.92.179/ws/smart-hydroponic/device",
    controlUrl: "ws://103.147.92.179/ws/smart-hydroponic/control",
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
let wsDashboard = null;
let wsControl = null;
let isConnectingDashboard = false;
let isConnectingControl = false;
let dashboardPingInterval = null;
let controlPingInterval = null;

// General retry wrapper
const retryConnection = (fn, delay = 5000) => {
    setTimeout(fn, delay);
};

// Connect to WebSocket (Reusable)
function connectWebSocket(url, handlers, type = 'generic') {
    let ws;
    try {
        ws = new WebSocket(url);
    } catch (err) {
        console.error(`[${type}] Failed to connect:`, err.message);
        retryConnection(() => connectWebSocket(url, handlers, type));
        return;
    }

    ws.onopen = () => {
        console.info(`[${type}] Connected`);
        handlers.onOpen?.(ws);

        // Start ping-pong mechanism
        if (type === "Dashboard") {
            clearInterval(dashboardPingInterval);
            dashboardPingInterval = setInterval(() => {
                if (ws.readyState === WebSocket.OPEN) {
                    ws.send(JSON.stringify({ type: "ping" }));
                }
            }, 30000); // Ping every 30 seconds
        } else if (type === "Control") {
            clearInterval(controlPingInterval);
            controlPingInterval = setInterval(() => {
                if (ws.readyState === WebSocket.OPEN) {
                    ws.send(JSON.stringify({ type: "ping" }));
                }
            }, 30000); // Ping every 30 seconds
        }
    };

    ws.onmessage = (event) => {
        handlers.onMessage?.(event, ws);
    };

    ws.onclose = (event) => {
        console.warn(`[${type}] Connection closed: ${event.reason}`);
        handlers.onClose?.();

        // Clear ping interval on close
        if (type === "Dashboard") {
            clearInterval(dashboardPingInterval);
        } else if (type === "Control") {
            clearInterval(controlPingInterval);
        }

        retryConnection(() => connectWebSocket(url, handlers, type));
    };

    ws.onerror = (err) => {
        console.error(`[${type}] Error:`, err.message || err);
        ws.close();
    };

    return ws;
}

function connectDashboardWebSocket() {
    if (isConnectingDashboard) return;
    isConnectingDashboard = true;

    wsDashboard = connectWebSocket(config.wsUrl, {
        onOpen: (ws) => {
            isConnectingDashboard = false;
            const registerPayload = {
                device_id: config.deviceId,
                type: "register"
            };
            ws.send(JSON.stringify(registerPayload));
        },
        onMessage: (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.type === "update_data") {
                    handleSensorData(data.data);
                } else if (data.type === "pong") {
                    console.info("[Dashboard] Pong received");
                }
            } catch (e) {
                console.error("Invalid message from dashboard:", event.data);
            }
        },
        onClose: () => {
            isConnectingDashboard = false;
        }
    }, "Dashboard");
}

function connectControlWebSocket() {
    if (isConnectingControl) return;
    isConnectingControl = true;

    wsControl = connectWebSocket(config.controlUrl, {
        onOpen: () => {
            isConnectingControl = false;
        },
        onMessage: (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.type === "pong") {
                    console.info("[Control] Pong received");
                }
            } catch (e) {
                console.error("Invalid message from control:", event.data);
            }
        },
        onClose: () => {
            isConnectingControl = false;
        }
    }, "Control");
}

function connectWebSockets() {
    connectDashboardWebSocket();
    connectControlWebSocket();
}


/**
 * Send command to control WebSocket
 */
function sendCommand() {
    if (wsControl && wsControl.readyState === WebSocket.OPEN) {
        wsControl.send(JSON.stringify(state));
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
