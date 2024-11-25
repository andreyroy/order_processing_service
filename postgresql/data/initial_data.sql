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
('order_nr_1', 'executor_id_1', '', '', NULL, NULL, NOW(), NULL, NULL, NULL);
