DROP SCHEMA IF EXISTS order_processing CASCADE;

CREATE SCHEMA IF NOT EXISTS order_processing;

CREATE TABLE IF NOT EXISTS order_processing.orders (
    order_nr TEXT PRIMARY KEY,
    executor_id TEXT,
    customer_id TEXT,
    zone_id TEXT,
    base_cost FLOAT,
    cost FLOAT,
    assigned_at timestamptz,
    acquired_at timestamptz,
    cancelled_at timestamptz,
    cancel_reason TEXT
);
