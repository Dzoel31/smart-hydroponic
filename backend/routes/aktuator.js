const express = require('express');
const db = require('../config/db');

const router = express.Router();
router.get('/', (req, res) => {
    res.send('Hello from actuators');
})

router.get('/actuators', (req, res) => {
    db.query('SELECT * FROM actuator_data', (err, result) => {
        if (err) {
            console.log('Error getting data: ', err);
            res.status(500).send('Error getting data');
            return;
        }

        res.status(200).json(result);
    });
});

router.get('/actuators/:actuator', (req, res) => {
    const actuator = req.params.actuator;

    db.query(`SELECT ${actuator},timestamp FROM actuator_data`, (err, result) => {
        if (err) {
            console.log('Error getting data: ', err);
            res.status(500).send('Error getting data');
            return;
        }

        res.status(200).json(result);
    });
});

module.exports = router;