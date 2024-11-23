DROP SCHEMA IF EXISTS order_processing CASCADE;

CREATE SCHEMA IF NOT EXISTS order_processing;

CREATE TABLE IF NOT EXISTS order_processing.users (
    name TEXT PRIMARY KEY,
    count INTEGER DEFAULT(1)
);
