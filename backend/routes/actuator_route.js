const express = require('express');

const {
    getAllDataActuator,
    getActuatorData,
    getLatestActuatorData,
} = require('../controller/actuator_controller');

const router = express.Router();

router.get('/', (req, res) => {
    res.send('Hello from actuators');
});

router.get('/actuators', getAllDataActuator);
router.get('/actuators/:actuator', getActuatorData);
router.get('/actuators/:actuator/latest', getLatestActuatorData);

module.exports = router;
