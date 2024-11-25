import pytest
from testsuite.databases import pgsql


@pytest.fixture
def order_acquire_body():
    return {
        "ExecutorID": "executor_id_1"
    }


# Мокируем внешние сервисы: order-info, executor-info, zone-info, toll-road-info
@pytest.fixture
def mock_order_info(mockserver):
    @mockserver.json_handler("/api/order-info")
    async def handler(request):
        order_id = request.args["order_id"]
        if order_id == "order_nr_1":
            return {
                "id": "order_nr_1",
                "zone_id": "zone_1",
                "customer_id": "customer_123",
                "base_cost": 100.0,
            }
        return mockserver.make_response("Not Found", status=404)


@pytest.fixture
def mock_executor_info(mockserver):
    @mockserver.json_handler("/api/executor-info")
    async def handler(request):
        executor_id = request.args["executor_id"]
        if executor_id == "executor_id_1":
            return {
                "id": "executor_id_1",
                "tags": ["reliable", "fast"],
                "rating": 4.8,
            }
        return mockserver.make_response("Not Found", status=404)


@pytest.fixture
def mock_zone_info(mockserver):
    @mockserver.json_handler("/api/zone-info")
    async def handler(request):
        zone_id = request.args["zone_id"]
        if zone_id == "zone_1":
            return {
                "id": "zone_1",
                "name": "Central",
                "coef": 1.1,
            }
        return mockserver.make_response("Not Found", status=404)


@pytest.fixture
def mock_toll_road_info(mockserver):
    @mockserver.json_handler("/api/toll-road-info")
    async def handler(request):
        zone_id = request.args["zone_id"]
        if zone_id == "zone_1":
            return {
                "id": "toll_road_1",
                "zone_id": "zone_1",
                "cost": 15.0,
            }
        return mockserver.make_response("Not Found", status=404)


# Тесты

@pytest.mark.pgsql("order_processing", files=['db_1.sql', 'initial_data.sql'])
async def test_order_acquire_success(
    service_client,
    mock_order_info,
    mock_executor_info,
    mock_zone_info,
    mock_toll_road_info,
):
    response = await service_client.get(
        "/v1/order/acquire",
        params={"ExecutorID": "executor_id_1"},
    )

    assert response.status == 200
    response_json = response.json()

    assert response_json["status"] == "success"
    assert response_json["data"]["order"]["id"] == "order_nr_1"
    assert response_json["data"]["executor"]["id"] == "executor_id_1"
    assert response_json["data"]["zone"]["id"] == "zone_1"
    assert response_json["data"]["toll_road"]["id"] == "toll_road_1"
    assert response_json["data"]["total_cost"] == pytest.approx(125.0)


@pytest.mark.pgsql("order_processing", files=['db_1.sql', 'initial_data.sql'])
async def test_order_acquire_bad_request(service_client):
    response = await service_client.get(
        "/v1/order/acquire",
        params={}
    )
    assert response.status == 400
    assert response.json()["message"] == "Missing executor id in query"


@pytest.mark.pgsql("order_processing", files=['db_1.sql', 'initial_data.sql'])
async def test_order_acquire_order_not_found(
    service_client,
    mock_order_info,
    mock_executor_info,
    mock_zone_info,
    mock_toll_road_info,
):
    response = await service_client.get(
        "/v1/order/acquire",
        params={"ExecutorID": "executor_id_1"},
    )

    assert response.status == 404
    assert response.json()["message"] == "No orders for executor"


@pytest.mark.pgsql("order_processing", files=['initial_data.sql'])
async def test_order_acquire_executor_not_found(
    service_client,
    mock_order_info,
    mock_executor_info,
    mock_zone_info,
    mock_toll_road_info,
):
    response = await service_client.get(
        "/v1/order/acquire",
        params={"ExecutorID": "unknown_executor_id"},
    )

    assert response.status == 404
    assert response.json()["message"] == "Executor info not found for executor_id: unknown_executor_id"
