const express = require('express');

const {
    addActuatorData,
    getAllDataActuator,
    getActuatorData,
    getLatestActuatorData,
} = require('../controller/actuator_controller');

const router = express.Router();

router.get('/api/smart-hydroponic/v1/actuators/hello', (req, res) => {
    res.send('Hello from actuators');
});

router.post('/api/smart-hydroponic/v1/actuators', addActuatorData);
router.get('/api/smart-hydroponic/v1/actuators', getAllDataActuator);
router.get('/api/smart-hydroponic/v1/actuator/latest', getLatestActuatorData); 
router.get('/api/smart-hydroponic/v1/actuator/:actuator', getActuatorData);

module.exports = router;
