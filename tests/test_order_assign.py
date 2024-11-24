import pytest

from testsuite.databases import pgsql


async def test_order_create_ok(service_client):
    response = await service_client.post(
        "/v1/order/assign",
        json={"order_nr": "order_nr_2", "executor_id": "executor_id_1"},
    )

    assert response.status == 200
    assert response.json() == {"message": "success"}

    response = await service_client.post(
        "/v1/order/assign",
        json={"order_nr": "order_nr_2", "executor_id": "executor_id_1"},
    )

    assert response.status == 409
    assert response.json() == {"message": "order already exists"}


async def test_order_create_bad_request(service_client):
    response = await service_client.post(
        "/v1/order/assign", json={"order_nr": "order_nr_2"}
    )

    assert response.status == 400
    assert response.json() == {"message": "missing executor_id in body"}

    response = await service_client.post(
        "/v1/order/assign", json={"executor_id": "executor_id_1"}
    )
    assert response.status == 400
    assert response.json() == {"message": "missing order_nr in body"}
