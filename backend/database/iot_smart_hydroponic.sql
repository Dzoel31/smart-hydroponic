--
-- PostgreSQL database dump
--

-- Dumped from database version 17.3
-- Dumped by pg_dump version 17.3

SET statement_timeout = 0;

SET lock_timeout = 0;

SET idle_in_transaction_session_timeout = 0;

SET client_encoding = 'UTF8';

SET standard_conforming_strings = ON;

SET search_path TO public;

SET check_function_bodies = FALSE;

SET xmloption = CONTENT;

SET client_min_messages = warning;

SET row_security = OFF;

SET TIMEZONE = 'Asia/Jakarta';

SET default_tablespace = '';

CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

SET default_table_access_method = heap;
--
-- Name: actuator_data; Type: TABLE; Schema: public; Owner: admin_iot_hidroponik
--

DROP TABLE IF EXISTS public.actuator_data CASCADE;

CREATE TABLE IF NOT EXISTS public.actuator_data (
    pumpstatus INTEGER NOT NULL,
    lightstatus INTEGER NOT NULL,
    "timestamp" timestamptz DEFAULT CURRENT_TIMESTAMP NOT NULL
);

ALTER TABLE IF EXISTS public.actuator_data OWNER TO admin_iot_hidroponik;

--
-- Name: sensor_data; Type: TABLE; Schema: public; Owner: admin_iot_hidroponik
--

DROP TABLE IF EXISTS public.sensor_data CASCADE;

CREATE TABLE IF NOT EXISTS public.sensor_data (
    moisture1 integer NOT NULL,
    moisture2 integer NOT NULL,
    moisture3 integer NOT NULL,
    moisture4 integer NOT NULL,
    moisture5 integer NOT NULL,
    moisture6 integer NOT NULL,
    moistureavg integer NOT NULL,
    flowrate double precision NOT NULL,
    totallitres double precision NOT NULL,
    distancecm double precision NOT NULL,
    "timestamp" timestamptz DEFAULT CURRENT_TIMESTAMP NOT NULL
);

ALTER TABLE IF EXISTS public.sensor_data OWNER TO admin_iot_hidroponik;

--
-- Name: environment_data; Type: TABLE; Schema: public; Owner: admin_iot_hidroponik
--

DROP TABLE IF EXISTS public.environment_data CASCADE;

CREATE TABLE IF NOT EXISTS public.environment_data (
    temperature_atas double precision NOT NULL,
    humidity_atas integer NOT NULL,
    temperature_bawah double precision NOT NULL,
    humidity_bawah integer NOT NULL,
    tds integer NOT NULL,
    "timestamp" timestamptz DEFAULT CURRENT_TIMESTAMP NOT NULL
);

ALTER TABLE IF EXISTS public.environment_data OWNER TO admin_iot_hidroponik;

SELECT public.create_hypertable (
        'public.sensor_data', 'timestamp', if_not_exists => TRUE
    );

SELECT public.create_hypertable (
        'public.actuator_data', 'timestamp', if_not_exists => TRUE
    );

SELECT public.create_hypertable (
        'public.environment_data', 'timestamp', if_not_exists => TRUE
    );

ALTER TABLE public.sensor_data
SET (
        timescaledb.compress,
        timescaledb.compress_segmentby = 'timestamp'
    );

ALTER TABLE public.actuator_data
SET (
        timescaledb.compress,
        timescaledb.compress_segmentby = 'timestamp'
    );

ALTER TABLE public.environment_data
SET (
        timescaledb.compress,
        timescaledb.compress_segmentby = 'timestamp'
    );

SELECT public.add_compression_policy (
        'public.sensor_data', INTERVAL '1 month', if_not_exists => TRUE
    );

SELECT public.add_compression_policy (
        'public.actuator_data', INTERVAL '1 month', if_not_exists => TRUE
    );

SELECT public.add_compression_policy (
        'public.environment_data', INTERVAL '1 month', if_not_exists => TRUE
    );

--
-- PostgreSQL database dump complete
--