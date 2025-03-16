const express = require('express');
const {
    getAllDataSensor,
    getSensorData,
    getLatestSensorData,
} = require('../controller/sensor_controller');

const router = express.Router();

router.get('/', (req, res) => {
    res.send('Hello from sensors');
});

router.get('/sensors', getAllDataSensor);
router.get('/sensors/:sensor', getSensorData);
router.get('/sensors/:sensor/latest', getLatestSensorData);

module.exports = router;
