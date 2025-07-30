/* 
Controller for actuator
*/

const db = require('../config/db');

const { errorLogger } = require("../utils/logger");

const addActuatorData = async (req, res) => {
    try {
         const { device_id, data } = req.body;

        if (!device_id || !data) {
            return res.status(400).json({ message: 'Device ID and data are required' });
        }

        const query = `INSERT INTO actuator_data (deviceid, pump_status, light_status, automation_status) VALUES ($1, $2, $3, $4)`;
        const values = [
            device_id,
            data.pumpStatus,
            data.lightStatus,
            data.automationStatus,
        ];
        await db.query(query, values);
        res.status(201).json({
            status: 'success',
            message: 'Actuator data added successfully'
        });
    } catch (error) {
        errorLogger.error('Error adding actuator data: ', error);
        res.status(500).json({
            status: 'error',
            message: 'Error adding actuator data',
            error: error.message
        });
    }
};

const getAllDataActuator = async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 25;
        const offset = (page - 1) * limit;

        const result = await db.query('SELECT * FROM actuator_data LIMIT $1 OFFSET $2', [limit, offset]);
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
}

const getActuatorData = async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 25;
        const offset = (page - 1) * limit;
        const actuator = req.params.actuator;

        const result = await db.query(`SELECT ${actuator}, timestamp FROM actuator_data LIMIT $1 OFFSET $2`, [limit, offset]);
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
}

const getLatestActuatorData = async (req, res) => {
    try {
        const result = await db.query(`SELECT * FROM actuator_data ORDER BY timestamp DESC LIMIT 1`);
        res.status(200).json({
            message: 'Success',
            data: result.rows
        });
    } catch (error) {
        errorLogger.error('Error getting data: ', error);
        res.status(500).json({ 
            message: 'Error getting data', 
            error: error.message 
        });
    }
}

module.exports = {
    addActuatorData,
    getAllDataActuator,
    getActuatorData,
    getLatestActuatorData,
};
