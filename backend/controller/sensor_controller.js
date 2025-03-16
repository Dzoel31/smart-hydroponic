/* 
Controller for sensor data
*/

const db = require('../config/db');

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
        const sensor = req.params.sensor;

        const result = await db.query(`SELECT ${sensor}, timestamp FROM sensor_data ORDER BY timestamp DESC LIMIT 1`);
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
    getAllDataSensor,
    getSensorData,
    getLatestSensorData
};
