#include "handler.hpp"
#include "../../external/order_info_client.hpp"
#include "../../external/executor_info_client.hpp"
#include "../../external/zone_info_client.hpp"
#include "../../external/toll_road_info_client.hpp"

namespace handlers::order::acquire {

// SQL-запросы
const userver::storages::postgres::Query kFetchUnaquiredOrdersByExecutor{
    "SELECT order_nr FROM order_processing.orders WHERE executor_id=$1 AND acquired_at IS NULL "
    "ORDER BY assigned_at ASC LIMIT 1",
    userver::storages::postgres::Query::Name{"fetch_unaquired_orders_by_executor"},
};

const userver::storages::postgres::Query kUpdateAquired{
    "UPDATE order_processing.orders "
    "SET acquired_at = NOW() WHERE order_nr = $1",
    userver::storages::postgres::Query::Name{"update_acquired"},
};

const userver::storages::postgres::Query kInsertOrder{
    "INSERT INTO order_processing.orders ("
    "    order_nr, executor_id, customer_id, zone_id, base_cost, cost, assigned_at, acquired_at, "
    "    cancelled_at, cancel_reason) "
    "VALUES ($1, $2, $3, $4, $5, $6, NOW(), NULL, NULL, NULL) "
    "ON CONFLICT (order_nr) DO NOTHING",
    userver::storages::postgres::Query::Name{"insert_order"},
};

std::string Handler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {

    userver::formats::json::ValueBuilder result;

    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);

    const auto executor_id = request.GetArg("ExecutorID");
    if (executor_id.empty()) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
        result["message"] = "Missing executor id in query";
        return userver::formats::json::ToString(result.ExtractValue());
    }

    auto transaction = pg_cluster_->Begin(
        "acquire_order_transaction", userver::storages::postgres::ClusterHostType::kMaster, {});

    try {
        auto res = transaction.Execute(kFetchUnaquiredOrdersByExecutor, executor_id);

        if (res.IsEmpty()) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
            result["message"] = "No orders for executor";
            return userver::formats::json::ToString(result.ExtractValue());
        }

        const auto& row = res[0];
        const std::string order_nr = row["order_nr"].As<std::string>();

        transaction.Execute(kUpdateAquired, order_nr);

        transaction.Commit();

        ::external::order_info::OrderInfoClient order_info_client(http_client_);
        auto order_info = order_info_client.FetchOrderInfo(order_nr);
        if (!order_info.has_value()) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
            result["message"] = "No order info for order_nr";
            return userver::formats::json::ToString(result.ExtractValue());
        }

        external::executor_info::ExecutorInfoClient executor_info_client(http_client_);
        auto executor_info = executor_info_client.FetchExecutorInfo(executor_id);
        if (!executor_info) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
            result["message"] = "Executor info not found for executor_id: " + executor_id;
            return userver::formats::json::ToString(result.ExtractValue());
        }

        external::zone_info::ZoneInfoClient zone_info_client(http_client_);
        auto zone_info = zone_info_client.FetchZoneInfo(order_info->zone_id);
        if (!zone_info) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
            result["message"] = "Zone info not found for zone_id: " + order_info->zone_id;
            return userver::formats::json::ToString(result.ExtractValue());
        }

        external::toll_road_info::TollRoadInfoClient toll_road_info_client(http_client_);
        auto toll_road_info = toll_road_info_client.FetchTollRoadInfo(order_info->zone_id);
        if (!toll_road_info) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
            result["message"] = "Toll road not found for zone_id: " + order_info->zone_id;
            return userver::formats::json::ToString(result.ExtractValue());
        }

        float total_cost = order_info->base_cost * zone_info->coef + toll_road_info->cost;

        auto insert_transaction = pg_cluster_->Begin(
            "insert_order_transaction", userver::storages::postgres::ClusterHostType::kMaster, {});

        insert_transaction.Execute(kInsertOrder,
                                   order_nr,
                                   executor_id,
                                   order_info->customer_id,
                                   order_info->zone_id,
                                   order_info->base_cost,
                                   total_cost); 

        insert_transaction.Commit();

    userver::formats::json::ValueBuilder tags_json;
    for (const auto& tag : executor_info->tags) {
        tags_json.PushBack(tag);
    }

        // Формируем успешный ответ
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
        result["status"] = "success";
        result["data"] = userver::formats::json::MakeObject(
            "order", userver::formats::json::MakeObject(
                         "id", order_info->id,
                         "zone_id", order_info->zone_id,
                         "customer_id", order_info->customer_id,
                         "base_cost", order_info->base_cost),
            "executor", userver::formats::json::MakeObject(
                            "id", executor_info->id,
                            "tags", tags_json.ExtractValue(),
                            "rating", executor_info->rating),
            "zone", userver::formats::json::MakeObject(
                        "id", zone_info->id,
                        "name", zone_info->name,
                        "coef", zone_info->coef),
            "toll_road", userver::formats::json::MakeObject(
                             "id", toll_road_info->id,
                             "zone_id", toll_road_info->zone_id,
                             "cost", toll_road_info->cost),
            "total_cost", total_cost);
        return userver::formats::json::ToString(result.ExtractValue());

    } catch (const std::exception& ex) {
        transaction.Rollback();
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kInternalServerError);
        result["message"] = "An error occurred: " + std::string(ex.what());
        return userver::formats::json::ToString(result.ExtractValue());
    }
}

}  // namespace handlers::order::acquire