/**
 * Utility to handle fetch requests with error handling and optional JSON parsing
 */
async function apiRequest(url, options = {}, parseJson = true) {
    try {
        url = `http://localhost:15000${url}`;
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
    actuators: '/api/smart-hydroponic/v1/actuators',
    actuatorLatest: '/api/smart-hydroponic/v1/actuator/latest',
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
    avg_temperature: document.getElementById("avg_temperature"),
};

// State management
const state = {
    device_id: "dashboard-device",
    target_device_id: "esp8266-actuator-device",
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
    elements.pumpButton.disabled = elements.lampButton.disabled = state.data.automationStatus === 1;
    elements.otomationButton.checked = state.data.automationStatus === 1;
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
    const dataToFetch = [endpoints.sensorLatest, endpoints.environmentLatest, endpoints.actuatorLatest];
    try {
        for (const endpoint of dataToFetch) {
            const data = await apiRequest(endpoint);
            if (endpoint === endpoints.sensorLatest) {
                handleSensorData(data.data);
            } else if (endpoint === endpoints.environmentLatest) {
                handleEnvironmentData(data.data);
            } else if (endpoint === endpoints.actuatorLatest) {
                handleActuatorData(data.data);
            }
        }
    } catch (e) {
        console.error("Error fetching sensor data:", e);
    }
}

/**
 * Send current state as command to API
 */
async function sendCommand() {
    try {
        await apiRequest(endpoints.actuators, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(state)
        }, false);
    } catch (e) {}
}

// Event listeners
elements.otomationButton.addEventListener('change', () => {
    state.data.automationStatus = elements.otomationButton.checked ? 1 : 0;
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
updateUI();
setInterval(fetchSensorData, 5000);
