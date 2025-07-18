const express = require('express');
const cors = require('cors');
const dotenv = require('dotenv');

const router_sensor = require('./routes/sensor_route');
const router_actuator = require('./routes/actuator_route');
const router_environment = require('./routes/environment_route');
const router_auth = require('./routes/auth_route');

const db = require('./config/db');

const { deviceLogger, errorLogger } = require("./utils/logger");

db.connect((err) => {
    if (err) {
        console.log('Error connecting to database:', err.message);
        errorLogger.error('Error connecting to database:', err.message);
        return;
    }
    deviceLogger.info('Database connected');
});

dotenv.config();

const app = express();

app.use(cors());
app.use(express.json());
app.get('/', (req, res) => {
    res.send('Welcome to Smart Hydroponic API');
});

app.use(router_sensor);
app.use(router_actuator);
app.use(router_environment);
app.use('/', router_auth);


app.listen(process.env.PORT, process.env.HOST, () => {
    deviceLogger.info(`Server is running on port http://localhost:${process.env.PORT}`);
});
