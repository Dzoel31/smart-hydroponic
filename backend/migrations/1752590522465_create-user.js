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

    pgm.createTable('user_data', {
        userid: {
            type: 'varchar(50)',
            notNull: true,
        },
        username: {
            type: 'varchar(50)',
            notNull: true,
        },
        password: {
            type: 'varchar(255)',
            notNull: true,
        },
        timestamp: {
            type: 'timestamptz',
            notNull: true,
            default: pgm.func('current_timestamp'),
        }
    });
};

/**
 * @param pgm {import('node-pg-migrate').MigrationBuilder}
 * @param run {() => void | undefined}
 * @returns {Promise<void> | void}
 */
exports.down = (pgm) => {
    pgm.dropTable('user_data', {ifExists: true, cascade: true});
};
