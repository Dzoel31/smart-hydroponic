/**
 * Controller for environment data
*/

const db = require('../config/db');

const { errorLogger } = require("../utils/logger");

const addEnvironmentData = async (req, res) => {
    try {
        const { device_id, data } = req.body;

        if (!device_id || !data) {
            return res.status(400).json({ message: 'Device ID and data are required' });
        }

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
            avg_humidity
        ];
        await db.query(query, values);
        res.status(201).json({
            message: 'Environment data added successfully'
        });
    } catch (error) {
        errorLogger.error('Error adding environment data: ', error);
        res.status(500).json({ 
            message: 'Error adding environment data', 
            error: error.message 
        });
    }
};

const getAllDataEnvironment = async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 25;
        const offset = (page - 1) * limit;

        const result = await db.query('SELECT * FROM environment_data LIMIT $1 OFFSET $2', [limit, offset]);
        res.status(200).json({
            message: 'Success',
            data: result.rows,
            totalRows: result.rowCount
        });
    } catch (error) {
        errorLogger.error('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
};

const getEnvironmentData = async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 25;
        const offset = (page - 1) * limit;
        const environment = req.params.environment;

        const result = await db.query(`SELECT ${environment}, timestamp FROM environment_data LIMIT $1 OFFSET $2`, [limit, offset]);
        res.status(200).json({
            message: 'Success',
            data: result.rows,
            totalRows: result.rowCount
        });
    } catch (error) {
        errorLogger.error('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
};

const getLatestEnvironmentData = async (req, res) => {
    try {
        const result = await db.query('SELECT * FROM environment_data ORDER BY timestamp DESC LIMIT 1');
        res.status(200).json({
            message: 'Success',
            data: result.rows,
            totalRows: result.rowCount
        });
    } catch (error) {
        errorLogger.error('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
};

module.exports = {
    addEnvironmentData,
    getAllDataEnvironment,
    getEnvironmentData,
    getLatestEnvironmentData
};
