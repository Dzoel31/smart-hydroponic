/* 
Controller for sensor data
*/

const db = require('../config/db');

const addSensorData = async (req, res) => {
    try {
        const { device_id, data } = req.body;

        if (!device_id || !data) {
            return res.status(400).json({ message: 'Device ID and data are required' });
        }

        // Hitung moistureavg secara otomatis berdasarkan jumlah key moistureN
        const moistureKeys = Object.keys(data).filter(key => /^moisture\d+$/.test(key));
        const moistureValues = moistureKeys.map(key => Number(data[key])).filter(val => !isNaN(val));
        const moistureavg = moistureValues.length > 0
            ? moistureValues.reduce((sum, val) => sum + val, 0) / moistureValues.length
            : null;

        const query = `INSERT INTO sensor_data (deviceid, moisture1, moisture2, moisture3, moisture4, moisture5, moisture6, moistureavg, flowrate, total_litres, distance_cm, ph, tds) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)`; 
        const values = [
            device_id,
            data.moisture1,
            data.moisture2,
            data.moisture3,
            data.moisture4,
            data.moisture5,
            data.moisture6,
            moistureavg,
            data.flowrate,
            data.total_litres,
            data.distance_cm,
            data.ph,
            data.tds,
        ];
        await db.query(query, values);
        res.status(201).json({
            status: 'success',
            message: 'Sensor data added successfully'
        });
    } catch (error) {
        console.log('Error adding sensor data: ', error);
        res.status(500).json({
            status: 'error',
            message: 'Error adding sensor data',
            error: error.message
        });
    }
};

const getAllDataSensor = async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 25;
        const offset = (page - 1) * limit;

        const result = await db.query('SELECT * FROM sensor_data LIMIT $1 OFFSET $2', [limit, offset]);
        res.status(200).json({
            message: 'Success',
            data: result.rows,
            totalRows: result.rowCount
        });
    } catch (error) {
        console.log('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
}

const getSensorData = async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 25;
        const offset = (page - 1) * limit;
        const sensor = req.params.sensor;

        const result = await db.query(`SELECT ${sensor}, timestamp FROM sensor_data LIMIT $1 OFFSET $2`, [limit, offset]);
        res.status(200).json({
            message: 'Success',
            data: result.rows,
            totalRows: result.rowCount
        });
    } catch (error) {
        console.log('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
}

const getLatestSensorData = async (req, res) => {
    try {
        const result = await db.query(`SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 1`);
        res.status(200).json({
            message: 'Success',
            data: result.rows
        });
    } catch (error) {
        console.log('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
}

module.exports = {
    addSensorData,
    getAllDataSensor,
    getSensorData,
    getLatestSensorData
};
