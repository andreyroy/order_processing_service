import pytest

from testsuite.databases import pgsql


async def test_order_cancel_ok(service_client):
    await service_client.post(
        "/v1/order/assign",
        json={"order_nr": "order_nr_1", "executor_id": "executor_id_1"},
    )

    response = await service_client.post(
        "/v1/order/cancel",
        json={
            "order_nr": "order_nr_1",
            "executor_id": "executor_id_1",
            "reason": "shit order! >:(",
        },
    )

    assert response.status == 200
    assert response.json() == {"message": "success"}


async def test_order_cancel_missing_order(service_client):
    response = await service_client.post(
        "/v1/order/cancel",
        json={
            "order_nr": "order_nr_1",
            "executor_id": "executor_id_1",
            "reason": "shit order! >:(",
        },
    )

    assert response.status == 404
    assert response.json() == {"message": "order not found"}


async def test_order_cancel_cancelled_order(service_client):
    await service_client.post(
        "/v1/order/assign",
        json={"order_nr": "order_nr_1", "executor_id": "executor_id_1"},
    )

    await service_client.post(
        "/v1/order/cancel",
        json={
            "order_nr": "order_nr_1",
            "executor_id": "executor_id_1",
            "reason": "shit order! >:(",
        },
    )

    response = await service_client.post(
        "/v1/order/cancel",
        json={
            "order_nr": "order_nr_1",
            "executor_id": "executor_id_1",
            "reason": "shit order! >:(",
        },
    )

    assert response.status == 200
    assert response.json() == {"message": "order already canceled"}


async def test_order_cancel_bad_request(service_client):
    response = await service_client.post(
        "/v1/order/cancel", json={"order_nr": "order_nr_2"}
    )

    assert response.status == 400
    assert response.json() == {"message": "missing required parameters"}

    response = await service_client.post(
        "/v1/order/cancel", json={"executor_id": "executor_id_1"}
    )
    assert response.status == 400
    assert response.json() == {"message": "missing required parameters"}

    response = await service_client.post(
        "/v1/order/cancel", json={"reason": "shit order! >:("}
    )
    assert response.status == 400
    assert response.json() == {"message": "missing required parameters"}
