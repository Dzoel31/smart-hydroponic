const ws = new WebSocket("ws://172.23.0.188:10000");

const pumpButton = document.getElementById("toggleButton");
const lampButton = document.getElementById("toggleLampButton");
const otomationButton = document.getElementById("toggleOtomation");

const statusPump = document.getElementById("statusPump");
const statusLamp = document.getElementById("statusLamp");
const statusOtomation = document.getElementById("statusOtomation");
// const statusTemperature = document.getElementById("statusTemperature");
const flowRate = document.getElementById("flowRate");
const distanceCm = document.getElementById("distanceCm");
const totalLitres = document.getElementById("totalLitres");
const moisture1 = document.getElementById("moisture1")
const moisture2 = document.getElementById("moisture2")
const moisture3 = document.getElementById("moisture3")
const moisture4 = document.getElementById("moisture4")
const moisture5 = document.getElementById("moisture5")
const moisture6 = document.getElementById("moisture6")
const moistureAvg = document.getElementById("moistureAvg");
const temperature_atas = document.getElementById("temperature_atas");
const temperature_bawah = document.getElementById("temperature_bawah");


let otomationState = 1;
let pumpState = 0;
let lampState = 0;

ws.onopen = () => {
    console.log("WebSocket connection opened");
};

ws.onmessage = (event) => {
    console.log("Message from server:", event.data);
    
    try {
        const data = JSON.parse(event.data);
        statusPump.textContent = `Pump Status: ${data.pumpStatus}`;
        statusLamp.textContent = `Lamp Status: ${data.lightStatus}`;
        flowRate.textContent = `${(data.flowRate == undefined) ? 0 : data.flowRate}`;
        distanceCm.textContent = `${(data.distanceCm == undefined) ? 0 : data.distanceCm}`;
        
        totalLitres.textContent = `${(data.totalLitres == undefined) ? 0 : data.totalLitres}`;
        moisture1.textContent = `${(data.moisture1 == undefined) ? 0 : data.moisture1}`;
        moisture2.textContent = `${(data.moisture2 == undefined) ? 0 : data.moisture2}`;
        moisture3.textContent = `${(data.moisture3 == undefined) ? 0 : data.moisture3}`;
        moisture4.textContent = `${(data.moisture4 == undefined) ? 0 : data.moisture4}`;
        moisture5.textContent = `${(data.moisture5 == undefined) ? 0 : data.moisture5}`;
        moisture6.textContent = `${(data.moisture6 == undefined) ? 0 : data.moisture6}`;
        moistureAvg.textContent = `${(data.moistureAvg == undefined) ? 0 : data.moistureAvg}`;
        temperature_atas.textContent = `${(data.temperature_atas == undefined)? 0 : data.temperature_atas}`;
        temperature_bawah.textContent = `${(data.temperature_bawah == undefined) ? 0 : data.temperature_bawah}`;
        

    } catch (error) {
        console.error("Error parsing JSON:", error);
    }
};

ws.onclose = () => {
    console.log("WebSocket connection closed");
};

//kirim perintah otomasi
function sendOtomationCommand() {
    ws.send(JSON.stringify({type: "pump_light_ESP8266", otomationStatus: otomationState, pumpStatus: pumpState, lightStatus: lampState }))
}

// Kirim perintah pompa
function sendPumpCommand() {
    ws.send(JSON.stringify({ type: "pump_light_ESP8266", otomationStatus: otomationState, pumpStatus: pumpState, lightStatus: lampState }));
}

// Kirim perintah lampu
function sendLampCommand() {
    ws.send(JSON.stringify({ type: "pump_light_ESP8266", otomationStatus: otomationState, pumpStatus: pumpState, lightStatus: lampState }));
}

// Toggle otomation
otomationButton.addEventListener("click", () => {
    if (otomationState === 0) {
        otomationState = 1;
        otomationButton.textContent = "Otomatis";
        
        // Jika otomasi aktif, reset pompa dan lampu ke 0
        pumpState = 0;
        lampState = 0;
        pumpButton.disabled = true;
        lampButton.disabled = true;
        pumpButton.textContent = "Mati";
        lampButton.textContent = "Nyala";
        
        sendPumpCommand(); // Kirim status pompa dan lampu setelah reset
    } else {
        otomationState = 0;
        otomationButton.textContent = "Non Otomatis";
        pumpState = 0;
        lampState = 0;
        pumpButton.textContent = "Mati";
        lampButton.textContent = "Mati";
        // Jika otomasi nonaktif, tombol pompa dan lampu bisa digunakan kembali
        pumpButton.disabled = false;
        lampButton.disabled = false;
    }

    sendOtomationCommand();
});



// Toggle pompa
pumpButton.addEventListener("click", () => {
    if (pumpState === 0) {
        pumpState = 1;
        pumpButton.textContent = "Nyala";
    } else {
        pumpState = 0;
        pumpButton.textContent = "Mati";
    }
    sendPumpCommand();
});

// Toggle lampu
lampButton.addEventListener("click", () => {
    if (lampState === 0) {
        lampState = 1;
        lampButton.textContent = "Nyala";
    } else {
        lampState = 0;
        lampButton.textContent = "Mati";
    }
    sendLampCommand();
});
