/**
 * @type {import('node-pg-migrate').ColumnDefinitions | undefined}
 */
exports.shorthands = undefined;

/**
 * @param pgm {import('node-pg-migrate').MigrationBuilder}
 * @param run {() => void | undefined}
 * @returns {Promise<void> | void}
 */
exports.up = (pgm) => {
    pgm.renameColumn('sensor_data', 'flowRate', 'flowrate');
    pgm.renameColumn('sensor_data', 'totalLitres', 'total_litres');
    pgm.renameColumn('sensor_data', 'distanceCm', 'distance_cm');

    pgm.renameColumn('actuator_data', 'pumpStatus', 'pump_status');
    pgm.renameColumn('actuator_data', 'lightStatus', 'light_status');
    pgm.renameColumn('actuator_data', 'automationStatus', 'automation_status');
};

/**
 * @param pgm {import('node-pg-migrate').MigrationBuilder}
 * @param run {() => void | undefined}
 * @returns {Promise<void> | void}
 */
exports.down = (pgm) => {
    pgm.renameColumn('sensor_data', 'flowrate', 'flowRate');
    pgm.renameColumn('sensor_data', 'total_litres', 'totalLitres');
    pgm.renameColumn('sensor_data', 'distance_cm', 'distanceCm');
    
    pgm.renameColumn('actuator_data', 'pump_status', 'pumpStatus');
    pgm.renameColumn('actuator_data', 'light_status', 'lightStatus');
    pgm.renameColumn('actuator_data', 'automation_status', 'automationStatus');
};
