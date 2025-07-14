/*
Controller for environment routes
*/

const express = require('express');
const {
    addEnvironmentData,
    getAllDataEnvironment,
    getEnvironmentData,
    getLatestEnvironmentData,
} = require('../controller/environment_controller');

const router = express.Router();

router.get('/api/smart-hydroponic/v1/environments/hello', (req, res) => {
    res.send('Hello from environments');
});

router.post('/api/smart-hydroponic/v1/environments', addEnvironmentData);
router.get('/api/smart-hydroponic/v1/environments', getAllDataEnvironment);
router.get('/api/smart-hydroponic/v1/environment/latest', getLatestEnvironmentData); 
router.get('/api/smart-hydroponic/v1/environment/:environment', getEnvironmentData);

module.exports = router;
