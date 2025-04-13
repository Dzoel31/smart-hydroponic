const { Client } = require('pg');
const dotenv = require('dotenv');

dotenv.config();

const db = new Client();

module.exports = db;