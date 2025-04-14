const db = require('../config/db.js');

const plantDataUpdate = async (device_id, data) => {
    try {
        const query = `INSERT INTO sensor_data (deviceid, moisture1, moisture2, moisture3, moisture4, moisture5, moisture6, moistureavg, flowrate, total_litres, distance_cm) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)`;
        const values = [
            device_id,
            data.moisture1,
            data.moisture2,
            data.moisture3,
            data.moisture4,
            data.moisture5,
            data.moisture6,
            data.moistureAvg,
            data.flowRate,
            data.totalLitres,
            data.distanceCm,
        ];
        await db.query(query, values);
    } catch (error) {
        console.error('Error inserting plant data:', error);
    }
}

const environmentDataUpdate = async (device_id, data) => {
    try {
        const query = `INSERT INTO environment_data (deviceid, temperature_atas, temperature_bawah, avg_temperature, humidity_atas, humidity_bawah, avg_humidity) VALUES ($1, $2, $3, $4, $5, $6, $7)`;
        const values = [
            device_id,
            data.temperatureAtas,
            data.temperatureBawah,
            data.temperatureAvg,
            data.humidityAtas,
            data.humidityBawah,
            data.humidityAvg,
        ];
        await db.query(query, values);
    } catch (error) {
        console.error('Error inserting environment data:', error);
    }
}

const actuatorDataUpdate = async (device_id, data) => {
    try {
        const query = `INSERT INTO actuator_data (deviceid, pump_status, light_status, automation_status) VALUES ($1, $2, $3, $4)`;
        const values = [
            device_id,
            data.pumpStatus,
            data.lightStatus,
            data.automationStatus,
        ];
        await db.query(query, values);
    } catch (error) {
        console.error('Error inserting actuator data:', error);
    }
}

module.exports = {
    plantDataUpdate,
    environmentDataUpdate,
    actuatorDataUpdate
}