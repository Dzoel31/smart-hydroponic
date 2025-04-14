const express = require('express');

const {
    getAllDataActuator,
    getActuatorData,
    getLatestActuatorData,
} = require('../controller/actuator_controller');

const router = express.Router();

router.get('/api/smart-hydroponic/v1/actuators/hello', (req, res) => {
    res.send('Hello from actuators');
});

router.get('/api/smart-hydroponic/v1/actuators', getAllDataActuator);
router.get('/api/smart-hydroponic/v1/actuator/:actuator', getActuatorData);
router.get('/api/smart-hydroponic/v1/actuator/:actuator/latest', getLatestActuatorData);

module.exports = router;
