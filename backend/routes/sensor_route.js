const express = require('express');
const {
    addSensorData,
    getAllDataSensor,
    getSensorData,
    getLatestSensorData,
} = require('../controller/sensor_controller');

const router = express.Router();

router.get('/api/smart-hydroponic/v1/sensors/hello', (req, res) => {
    res.send('Hello from sensors');
});

router.post('/api/smart-hydroponic/v1/sensors', addSensorData);
router.get('/api/smart-hydroponic/v1/sensors', getAllDataSensor);
router.get('/api/smart-hydroponic/v1/sensor/latest', getLatestSensorData);
router.get('/api/smart-hydroponic/v1/sensor/:sensor', getSensorData);

module.exports = router;
