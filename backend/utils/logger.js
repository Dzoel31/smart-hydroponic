const { createLogger, format, transports } = require('winston');
require('winston-daily-rotate-file');
const path = require('path');
const fs = require('fs');
const archiver = require('archiver');
const cron = require('node-cron');

const getJakartaTime = () =>
    new Date().toLocaleString("id-ID", { timeZone: "Asia/Jakarta" });

const logsDir = path.join(__dirname, '../logs');
if (!fs.existsSync(logsDir)) {
    fs.mkdirSync(logsDir, { recursive: true });
}
const archiveDir = path.join(logsDir, 'archive');
if (!fs.existsSync(archiveDir)) {
    fs.mkdirSync(archiveDir, { recursive: true });
}

const getDaysInMonth = (year, month) =>
    new Date(year, month, 0).getDate();

function buildLogger(label) {
    const labelLower = label.toLowerCase();

    const textTransport = new transports.DailyRotateFile({
        filename: path.join(logsDir, `${labelLower}-%DATE%.log`),
        datePattern: 'YYYY-MM-DD',
        zippedArchive: true,
        maxSize: '100m',
        maxFiles: `${getDaysInMonth(new Date().getFullYear(), new Date().getMonth() + 1)}d`,
        auditFile: path.join(logsDir, `${labelLower}-audit.json`),
        level: 'info',
        format: format.printf(({ timestamp, level, message, label }) =>
            `[${timestamp}] [${label}] ${level.toUpperCase()}: ${message}`
        ),
    });

    const jsonTransport = new transports.DailyRotateFile({
        filename: path.join(logsDir, `${labelLower}-%DATE%.json`),
        datePattern: 'YYYY-MM-DD',
        zippedArchive: true,
        maxSize: '100m',
        maxFiles: `${getDaysInMonth(new Date().getFullYear(), new Date().getMonth() + 1)}d`,
        auditFile: path.join(logsDir, `${labelLower}-audit.json`),
        level: 'info',
        format: format.json(),
    });

    function moveLogOnRotate(oldFilename) {
        const now = new Date();
        const monthYear = `${now.toLocaleString('default', { month: 'long' })}-${now.getFullYear()}`;
        const targetDir = path.join(logsDir, monthYear);

        if (!fs.existsSync(targetDir)) {
            fs.mkdirSync(targetDir, { recursive: true });
        }

        const newFilePath = path.join(targetDir, path.basename(oldFilename));
        fs.cp(oldFilename, newFilePath, (err) => {
            if (err) console.error(`Gagal memindahkan log: ${err.message}`);
        });
    }

    textTransport.on('rotate', moveLogOnRotate);
    jsonTransport.on('rotate', moveLogOnRotate);

    return createLogger({
        format: format.combine(
            format.label({ label }),
            format.timestamp({ format: getJakartaTime }),
            format.errors({ stack: true }),
            format.splat()
        ),
        transports: [
            textTransport,
            jsonTransport,
            new transports.Console({
                format: format.combine(
                    format.colorize(),
                    format.printf(({ timestamp, level, message, label }) =>
                        `[${timestamp}] [${label}] ${level}: ${message}`
                    )
                ),
            }),
        ],
    });
}

// Archive logs every month
const archiveLogs = () => {
    const now = new Date();
    now.setMonth(now.getMonth() - 1); // Set to last month
    const monthYear = `${now.toLocaleString('default', { month: 'long' })}-${now.getFullYear()}`;
    const targetDir = path.join(logsDir, monthYear);
    const archiveFilePath = path.join(archiveDir, `${monthYear}.zip`);

    if (!fs.existsSync(targetDir)) {
        console.log(`Direktori ${targetDir} tidak ada.`);
        return;
    }

    if (fs.existsSync(archiveFilePath)) {
        console.log(`File ${archiveFilePath} sudah ada.`);
        return;
    }

    const output = fs.createWriteStream(archiveFilePath);
    const archive = archiver('zip', { zlib: { level: 9 } });

    output.on('close', () => {
        console.log(`Log ${monthYear} has been archived to ${archiveFilePath}: ${archive.pointer()} total bytes`);
        fs.rm(targetDir, { recursive: true, force: true }, (err) => {
            if (err) console.error(`Failed to delete directory: ${err.message}`);
        });
    });
    archive.pipe(output);
    archive.directory(targetDir, false);
    archive.finalize();
}

cron.schedule('0 0 1 * *', () => {
    archiveLogs();
}, {
    scheduled: true,
    timezone: "Asia/Jakarta"
});

module.exports = {
    deviceLogger: buildLogger('Device'),
    dashboardLogger: buildLogger('Dashboard'),
    errorLogger: buildLogger('Error'),
};
