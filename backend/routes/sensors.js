const express = require('express');
const db = require('../config/db');

const router = express.Router();

router.get('/', (req, res) => {
    res.send('Hello from sensors');
});

router.get('/sensors', (req, res) => {
    /**
     * Retrieves the limit from the query parameters and parses it as an integer.
     * If the limit is not provided or cannot be parsed, defaults to 25.
    *
    * @constant {number} limit - The maximum number of items to retrieve.
    * @default 25
    */
    const page = parseInt(req.query.page) || 1;
    const limit = parseInt(req.query.limit) || 25;
    const offset = (page - 1) * limit;

    db.query('SELECT * FROM sensor_data LIMIT ? OFFSET ?', [limit, offset], (err, result) => {
        if (err) {
            console.log('Error getting data: ', err);
            res.status(500).send('Error getting data');
            return;
        }

        res.status(200).json(result);
    });
});

router.get('/sensors/:sensor', (req, res) => {
    /**
     * Parses the 'limit' query parameter from the request and sets a default value if not provided.
     * 
     * @param {Object} req - The request object.
     * @param {Object} req.query - The query parameters of the request.
     * @param {string} req.query.limit - The limit query parameter as a string.
     * @returns {number} The parsed limit value or the default value of 25.
     */
    const page = parseInt(req.query.page) || 1;
    const limit = parseInt(req.query.limit) || 25;
    const offset = (page - 1) * limit;
    const sensor = req.params.sensor;

    db.query(`SELECT ${sensor},timestamp FROM sensor_data LIMIT ? OFFSET ?`, [limit, offset], (err, result) => {
        if (err) {
            console.log('Error getting data: ', err);
            res.status(500).send('Error getting data');
            return;
        }

        res.status(200).json(result);
    });
});

module.exports = router;