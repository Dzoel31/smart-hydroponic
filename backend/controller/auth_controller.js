const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const { jwtSecret, jwtExpiresIn } = require('../config/config');
const User = require('../models/user_model');

const { errorLogger } = require("../utils/logger");

// Fungsi generate token JWT
const generateToken = (user) => {
    return jwt.sign({ id: user.id }, jwtSecret, { expiresIn: jwtExpiresIn });
};

exports.register = async (req, res) => {
    try {
        const { username, password } = req.body;
        const hashedPassword = await bcrypt.hash(password, 10);

        const result = await User.create(username, hashedPassword);

        res.json({ message: 'User registered successfully' });
    } catch (err) {
        errorLogger.error("[REGISTER] Error:", err);  // << Tampilkan pesan error detail
        res.status(500).json({ message: 'Internal server error' });
    }
};

exports.login = async (req, res) => {
    try {
        const { username, password } = req.body;
        const user = await User.findByUsername(username);
        if (!user)
            return res.status(400).json({ message: 'User not found' });

        const match = await bcrypt.compare(password, user.password);
        if (!match)
            return res.status(401).json({ message: 'Wrong password' });

        const token = generateToken(user);
        res.json({ message: 'Login successful', token });
    } catch (err) {
        errorLogger.error("[LOGIN] Error:", err);
        res.status(500).json({ message: 'Internal server error' });
    }
};