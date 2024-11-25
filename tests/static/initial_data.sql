INSERT INTO order_processing.orders (
    order_nr,
    executor_id,
    customer_id,
    zone_id,
    base_cost,
    cost,
    assigned_at,
    acquired_at,
    cancelled_at,
    cancel_reason
) VALUES 
('order_nr_1', NULL, 'customer_1', 'zone_1', 100.0, NULL, NOW(), NULL, NULL, NULL);
