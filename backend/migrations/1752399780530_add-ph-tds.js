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
    pgm.addColumns('sensor_data', {
        ph: {
            type: 'double precision',
            notNull: true,
            default: 0,
        },
        tds: {
            type: 'double precision',
            notNull: true,
            default: 0,
        },
    });

    pgm.alterColumn('sensor_data', 'moisture1', {
        default: 0,
    })
    pgm.alterColumn('sensor_data', 'moisture2', {
        default: 0,
    });
    pgm.alterColumn('sensor_data', 'moisture3', {
        default: 0,
    });
    pgm.alterColumn('sensor_data', 'moisture4', {
        default: 0,
    });
    pgm.alterColumn('sensor_data', 'moisture5', {
        default: 0,
    });
    pgm.alterColumn('sensor_data', 'moisture6', {
        default: 0,
    });
    pgm.alterColumn('sensor_data', 'moistureavg', {
        default: 0.0,
    });
    pgm.alterColumn('sensor_data', 'flowrate', {
        default: 0.0,
    });
    pgm.alterColumn('sensor_data', 'total_litres', {
        default: 0.0,
    });
    pgm.alterColumn('sensor_data', 'distance_cm', {
        default: 0.0,
    });

    pgm.alterColumn('environment_data', 'temperature_atas', {
        default: 0.0,
    });
    pgm.alterColumn('environment_data', 'temperature_bawah', {
        default: 0.0,
    });
    pgm.alterColumn('environment_data', 'avg_temperature', {
        default: 0.0,
    });
    pgm.alterColumn('environment_data', 'humidity_atas', {
        default: 0.0,
    });
    pgm.alterColumn('environment_data', 'humidity_bawah', {
        default: 0.0,
    });
    pgm.alterColumn('environment_data', 'avg_humidity', {
        default: 0.0,
    });
};

/**
 * @param pgm {import('node-pg-migrate').MigrationBuilder}
 * @param run {() => void | undefined}
 * @returns {Promise<void> | void}
 */
exports.down = (pgm) => {
    pgm.dropColumns('sensor_data', ['ph', 'tds']);
};
