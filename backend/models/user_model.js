const db = require('../config/db');
const { v4: uuidv4 } = require('uuid');


module.exports = {
    findByUsername: async (username) => {
        const res = await db.query('SELECT * FROM user_data WHERE username = $1', [username]);
        return res.rows[0];
    },
    findById: async (userid) => {
        const res = await db.query('SELECT * FROM user_data WHERE userid = $1', [userid]);
        return res.rows[0];
    },
    create: async (username, password) => {
        const userid = uuidv4();
        return await db.query(
            'INSERT INTO user_data (userid, username, password) VALUES ($1, $2, $3)',
            [userid, username, password]
        );
    }
};