const express = require('express');
const db = require('../config/db');

const router = express.Router();

router.get('/', (req, res) => {
    res.send('Hello from sensors');
});

router.get('/sensors', (req, res) => {
    db.query('SELECT * FROM sensor_data', (err, result) => {
        if (err) {
            console.log('Error getting data: ', err);
            res.status(500).send('Error getting data');
            return;
        }

        res.status(200).json(result);
    });
});

router.get('/sensors/:sensor', (req, res) => {
    const sensor = req.params.sensor;

    db.query(`SELECT ${sensor} FROM sensor_data`, (err, result) => {
        console.log(result);
        if (err) {
            console.log('Error getting data: ', err);
            res.status(500).send('Error getting data');
            return;
        }

        res.status(200).json(result);
    });
});

module.exports = router;