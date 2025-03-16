const elements = {
    pumpButton: document.getElementById("togglePumpButton"),
    lampButton: document.getElementById("toggleLampButton"),
    otomationButton: document.getElementById("toggleAutomation"),
    statusPump: document.getElementById("statusPump"),
    statusLamp: document.getElementById("statusLamp"),
    statusOtomation: document.getElementById("statusOtomation"),
    flowRate: document.getElementById("flowRate"),
    distanceCm: document.getElementById("distanceCm"),
    totalLitres: document.getElementById("totalLitres"),
    moisture: [
        document.getElementById("moisture1"),
        document.getElementById("moisture2"),
        document.getElementById("moisture3"),
        document.getElementById("moisture4"),
        document.getElementById("moisture5"),
        document.getElementById("moisture6")
    ],
    moistureAvg: document.getElementById("moistureAvg"),
    temperature_atas: document.getElementById("temperature_atas"),
    temperature_bawah: document.getElementById("temperature_bawah"),
    humidity_atas: document.getElementById("humidity_atas"),
    humidity_bawah: document.getElementById("humidity_bawah"),
    avg_temperature: document.getElementById("avg_temperature")
};

let states = {
    otomationStatus: 1,
    pumpStatus: 0,
    lightStatus: 0
};

function createWebSocket(url, onOpen, onMessage, onClose) {
    let ws = new WebSocket(url);
    let pingInterval;

    ws.onopen = onOpen;

    ws.onmessage = onMessage;

    ws.onclose = () => {
        console.log(`WebSocket connection to ${url} closed, attempting to reconnect...`);
        clearInterval(pingInterval);
        setTimeout(() => {
            ws = createWebSocket(url, onOpen, onMessage, onClose);
        }, 5000);
    };

    return ws;
}

function handleWebSocketMessage(event, handlers) {
    console.log(`Message from ${handlers.name}:`, event.data);
    try {
        const data = JSON.parse(event.data);
        handlers.update(data);
    } catch (error) {
        console.error(`Error parsing JSON ${handlers.name}:`, error);
    }
}

const wsAktuator = createWebSocket(
    "ws://192.168.1.10:10000/actuator",
    () => {
        console.log("WebSocket aktuator connection opened");
        wsAktuator.send(JSON.stringify(states));
    },
    (event) => handleWebSocketMessage(event, {
        name: 'Aktuator',
        update: (data) => {
            states.pumpStatus = data.pumpStatus ?? 0;
            states.lightStatus = data.lightStatus ?? 0;
            elements.statusPump.textContent = `Pump Status: ${states.pumpStatus}`;
            elements.statusLamp.textContent = `Lamp Status: ${states.lightStatus}`;
            elements.pumpButton.checked = states.pumpStatus;
            elements.lampButton.checked = states.lightStatus;
        }
    })
);

const wsEnvData = createWebSocket(
    "ws://192.168.1.10:10000/environmentdata",
    () => console.log("WebSocket environmentdata connection opened"),
    (event) => handleWebSocketMessage(event, {
        name: 'EnvData',
        update: (data) => {
            elements.temperature_atas.textContent = data.temperature_atas ?? 0;
            elements.temperature_bawah.textContent = data.temperature_bawah ?? 0;
            elements.humidity_atas.textContent = data.humidity_atas ?? 0;
            elements.humidity_bawah.textContent = data.humidity_bawah ?? 0;
            elements.avg_temperature.textContent = (data.temperature_atas + data.temperature_bawah) / 2;
        }
    })
);

const wsPlantData = createWebSocket(
    "ws://192.168.1.10:10000/plantdata",
    () => console.log("WebSocket PlantData connection opened"),
    (event) => handleWebSocketMessage(event, {
        name: 'PlantData',
        update: (data) => {
            elements.flowRate.textContent = data.flowRate ?? 0;
            elements.distanceCm.textContent = data.distanceCm ?? 0;
            elements.totalLitres.textContent = data.totalLitres ?? 0;
            elements.moisture.forEach((el, index) => el.textContent = data[`moisture${index + 1}`] ?? 0);
            elements.moistureAvg.textContent = data.moistureAvg ?? 0;
        }
    })
);

function sendCommand() {
    wsAktuator.send(JSON.stringify(states));
}

if (states.otomationStatus === 1) {
    elements.pumpButton.disabled = true;
    elements.lampButton.disabled = true;
}

elements.otomationButton.addEventListener('change', () => {
    states.otomationStatus = elements.otomationButton.checked ? 1 : 0;
    elements.otomationButton.textContent = elements.otomationButton.checked ? "Otomatis" : "Non Otomatis";
    states.pumpStatus = 0;
    states.lightStatus = 0;
    elements.pumpButton.disabled = elements.otomationButton.checked;
    elements.lampButton.disabled = elements.otomationButton.checked;
    sendCommand();
});

elements.pumpButton.addEventListener("change", () => {
    states.pumpStatus = elements.pumpButton.checked ? 1 : 0;
    sendCommand();
});

elements.lampButton.addEventListener("change", () => {
    states.lightStatus = elements.lampButton.checked ? 1 : 0;
    sendCommand();
});
