const db = require('../config/db.js');
const { deviceLogger } = require("./logger.js");

const plantDataUpdate = async (device_id, data) => {
    try {
        const moistureKeys = Object.keys(data).filter(key => /^moisture\d+$/.test(key));
        const moistureValues = moistureKeys.map(key => Number(data[key])).filter(val => !isNaN(val));
        const moistureAvg = moistureValues.length > 0 ? moistureValues.reduce((sum, val) => sum + val) / moistureValues.length : 0;

        data.ph = data.ph ?? 0;
        data.tds = data.tds ?? 0;
        const query = `INSERT INTO sensor_data (deviceid, moisture1, moisture2, moisture3, moisture4, moisture5, moisture6, moistureavg, flowrate, total_litres, distance_cm, ph, tds) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)`;
        const values = [
            device_id,
            data.moisture1,
            data.moisture2,
            data.moisture3,
            data.moisture4,
            data.moisture5,
            data.moisture6,
            moistureAvg,
            data.flowrate,
            data.total_litres,
            data.distance_cm,
            data.ph,
            data.tds,
        ];
        await db.query(query, values);
    } catch (error) {
        deviceLogger.error('Error inserting plant data:', error);
    }
}

const environmentDataUpdate = async (device_id, data) => {
    try {
        avg_temperature = (data.temperatureAtas + data.temperatureBawah) / 2;
        avg_humidity = (data.humidityAtas + data.humidityBawah) / 2;
        const query = `INSERT INTO environment_data (deviceid, temperature_atas, temperature_bawah, avg_temperature, humidity_atas, humidity_bawah, avg_humidity) VALUES ($1, $2, $3, $4, $5, $6, $7)`;
        const values = [
            device_id,
            data.temperatureAtas,
            data.temperatureBawah,
            avg_temperature,
            data.humidityAtas,
            data.humidityBawah,
            avg_humidity,
        ];
        await db.query(query, values);
    } catch (error) {
        deviceLogger.error('Error inserting environment data:', error);
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
        deviceLogger.error('Error inserting actuator data:', error);
    }
}

module.exports = {
    plantDataUpdate,
    environmentDataUpdate,
    actuatorDataUpdate
}