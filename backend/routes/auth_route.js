const express = require('express');
const router = express.Router();
const auth = require('../controller/auth_controller');
const { authenticateJWT } = require('../middleware/auth_middleware');

router.post('/api/smart-hydroponic/v1/register', auth.register);
router.post('/api/smart-hydroponic/v1/login', auth.login);
router.get('/api/smart-hydroponic/v1/protected', authenticateJWT, (req, res) => {
    res.json({ message: 'This is protected', user: req.user });
});

module.exports = router;