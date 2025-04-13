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
    pgm.sql('CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;');

    pgm.createTable('sensor_data', {
        deviceid: {
            type: 'varchar(50)',
            notNull: true,
        },
        moisture1: {
            type: 'int',
            notNull: true,
        },
        moisture2: {
            type: 'int',
            notNull: true,
        },
        moisture3: {
            type: 'int',
            notNull: true,
        },
        moisture4: {
            type: 'int',
            notNull: true,
        },
        moisture5: {
            type: 'int',
            notNull: true,
        },
        moisture6: {
            type: 'int',
            notNull: true,
        },
        moistureavg: {
            type: 'double precision',
            notNull: true,
        },
        flowRate: {
            type: 'int',
            notNull: true,
        },
        totalLitres: {
            type: 'int',
            notNull: true,
        },
        distanceCm: {
            type: 'int',
            notNull: true,
        },
        timestamp: {
            type: 'timestamp',
            notNull: true,
            default: pgm.func('CURRENT_TIMESTAMP'),
        },
    });

    pgm.createTable('environment_data', {
        deviceid: {
            type: 'varchar(50)',
            notNull: true,
        },
        temperature_atas: {
            type: 'double precision',
            notNull: true,
        },
        temperature_bawah: {
            type: 'double precision',
            notNull: true,
        },
        avg_temperature: {
            type: 'double precision',
            notNull: true,
        },
        humidity_atas: {
            type: 'double precision',
            notNull: true,
        },
        humidity_bawah: {
            type: 'double precision',
            notNull: true,
        },
        avg_humidity: {
            type: 'double precision',
            notNull: true,
        },
        timestamp: {
            type: 'timestamp',
            notNull: true,
            default: pgm.func('CURRENT_TIMESTAMP'),
        },
    });

    pgm.createTable('actuator_data', {
        deviceid: {
            type: 'varchar(50)',
            notNull: true,
        },
        pumpStatus: {
            type: 'int',
            notNull: true,
        },
        lightStatus: {
            type: 'int',
            notNull: true,
        },
        automationStatus: {
            type: 'int',
            notNull: true,
        },
        timestamp: {
            type: 'timestamp',
            notNull: true,
            default: pgm.func('CURRENT_TIMESTAMP'),
        },
    });

    pgm.sql(`SELECT create_hypertable('sensor_data', 'timestamp', if_not_exists => TRUE);`);
    pgm.sql(`SELECT create_hypertable('environment_data', 'timestamp', if_not_exists => TRUE);`);
    pgm.sql(`SELECT create_hypertable('actuator_data', 'timestamp', if_not_exists => TRUE);`);

    pgm.sql(`ALTER TABLE sensor_data SET (timescaledb.compress, timescaledb.compress_segmentby = 'timestamp');`);
    pgm.sql(`ALTER TABLE environment_data SET (timescaledb.compress, timescaledb.compress_segmentby = 'timestamp');`);
    pgm.sql(`ALTER TABLE actuator_data SET (timescaledb.compress, timescaledb.compress_segmentby = 'timestamp');`);

    pgm.sql(`SELECT add_compression_policy('sensor_data', INTERVAL '1 month', if_not_exists => TRUE);`);
    pgm.sql(`SELECT add_compression_policy('environment_data', INTERVAL '1 month', if_not_exists => TRUE);`);
    pgm.sql(`SELECT add_compression_policy('actuator_data', INTERVAL '1 month', if_not_exists => TRUE);`);

};

/**
 * @param pgm {import('node-pg-migrate').MigrationBuilder}
 * @param run {() => void | undefined}
 * @returns {Promise<void> | void}
 */
exports.down = (pgm) => {
    pgm.dropTable('sensor_data', {ifExists: true, cascade: true});
    pgm.dropTable('environment_data', {ifExists: true, cascade: true});
    pgm.dropTable('actuator_data', {ifExists: true, cascade: true});
};
