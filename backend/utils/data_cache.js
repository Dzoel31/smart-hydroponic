const DEVICE_KEYS = [
    "esp32-plant-device",
    "esp32-environment-device",
    "esp8266-actuator-device",
];

const data_cache = Object.fromEntries(DEVICE_KEYS.map(key => [key, null]));

const updateCache = (device_id, data) => {
    if (DEVICE_KEYS.includes(device_id)) {
        data_cache[device_id] = { ...data };
    }
};

const getAlldata = () => {
    console.log("Payload data:", data_cache);
    return { ...data_cache };
};

const clearCache = () => {
    DEVICE_KEYS.forEach(key => {
        data_cache[key] = null;
    });
};

const isDataComplete = (data) => {
    const sensor_data = data["esp32-plant-device"];
    const environment_data = data["esp32-environment-device"];
    const actuator_data = data["esp8266-actuator-device"];

    if (!sensor_data || !environment_data || !actuator_data) {
        return false;
    }

    const requiredKeys = [
        "moisture1",
        "moisture2",
        "moisture3",
        "moisture4",
        "moisture5",
        "moisture6",
        "moistureAvg",
        "flowRate",
        "totalLitres",
        "distanceCm",
        "temperatureAtas",
        "temperatureBawah",
        "temperatureAvg",
        "humidityAtas",
        "humidityBawah",
        "humidityAvg",
        "pumpStatus",
        "lightStatus",
        "automationStatus",
    ];

    return requiredKeys.every(key => {
        const val = { ...sensor_data, ...environment_data, ...actuator_data }[key];
        return val !== undefined && val !== null;
    });
}

module.exports = {
    updateCache,
    getAlldata,
    clearCache,
    isDataComplete,
}