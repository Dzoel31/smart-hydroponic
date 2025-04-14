const express = require('express');
const {
    getAllDataSensor,
    getSensorData,
    getLatestSensorData,
} = require('../controller/sensor_controller');

const router = express.Router();

router.get('/api/smart-hydroponic/v1/sensors/hello', (req, res) => {
    res.send('Hello from sensors');
});

router.get('/api/smart-hydroponic/v1/sensors', getAllDataSensor);
router.get('/api/smart-hydroponic/v1/sensor/:sensor', getSensorData);
router.get('/api/smart-hydroponic/v1/sensor/:sensor/latest', getLatestSensorData);

module.exports = router;
