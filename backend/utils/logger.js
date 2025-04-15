const { createLogger, format, transports } = require('winston');
require('winston-daily-rotate-file');
const path = require('path');
const fs = require('fs');

const timezoned = () => {
    return new Date().toLocaleString("id-ID", { timeZone: "Asia/Jakarta" });
};

const logsDir = path.join(__dirname, '../logs');
if (!fs.existsSync(logsDir)) {
    fs.mkdirSync(logsDir, { recursive: true });
}

const buildLogger = (label) => {
    const labelLower = label.toLowerCase();

    return createLogger({
        format: format.combine(
            format.label({ label }),
            format.timestamp({ format: timezoned }),
            format.errors({ stack: true }),
            format.splat(),
        ),
        transports: [
            // Log versi TEXT
            new transports.DailyRotateFile({
                filename: path.join(logsDir, `${labelLower}-%DATE%.log`),
                datePattern: 'YYYY-MM-DD',
                zippedArchive: false,
                maxSize: '10m',
                maxFiles: '14d',
                level: 'info',
                format: format.combine(
                    format.printf(({ timestamp, level, message, label }) => {
                        return `[${timestamp}] [${label}] ${level.toUpperCase()}: ${message}`;
                    })
                ),
            }),

            // Log versi JSON
            new transports.DailyRotateFile({
                filename: path.join(logsDir, `${labelLower}-%DATE%.json`),
                datePattern: 'YYYY-MM-DD',
                zippedArchive: false,
                maxSize: '10m',
                maxFiles: '14d',
                level: 'info',
                format: format.combine(
                    format.json()
                ),
            }),

            // Console
            new transports.Console({
                format: format.combine(
                    format.colorize(),
                    format.printf(({ timestamp, level, message, label }) => {
                        return `[${timestamp}] [${label}] ${level}: ${message}`;
                    })
                ),
            }),
        ],
    });
};

module.exports = {
    deviceLogger: buildLogger('Device'),
    dashboardLogger: buildLogger('Dashboard'),
    errorLogger: buildLogger('Error'),
};
