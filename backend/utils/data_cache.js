const data_cache = {
    "esp32-plant-device": null,
    "esp32-environment-device": null,
    "esp8266-actuator-device": null,
};

const updateCache = (device_id, data) => {    
    data_cache[device_id] = {
        ...data,
    };

};

const getAlldata = () => {
    console.log("Payload data: ", data_cache);
    return data_cache;
};

const clearCache = () => {
    data_cache["esp32-plant-device"] = null;
    data_cache["esp32-environment-device"] = null;
    data_cache["esp8266-actuator-device"] = null;
}

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
        "humidityAtas",
        "humidityBawah",
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