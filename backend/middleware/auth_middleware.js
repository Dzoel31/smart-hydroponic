const jwt = require('jsonwebtoken');
const { jwtSecret } = require('../config/config');
const User = require('../models/user_model');

exports.authenticateJWT = async (req, res, next) => {
    const authHeader = req.headers.authorization;
    const token = authHeader && authHeader.split(' ')[1];
    
    if (!token) {
        return res.status(401).json({ message: 'Token not provided' });
    }

    try {
        const decoded = jwt.verify(token, jwtSecret);
        req.user = await User.findById(decoded.id);
        next();
    } catch (err) {
        res.status(403).json({ message: "Invalid token" });
    }
};