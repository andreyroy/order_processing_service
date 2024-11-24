#include "handler.hpp"

#include <userver/server/http/http_status.hpp>
#include <userver/storages/query.hpp>

namespace handlers::order::assign {

const userver::storages::postgres::Query kInsertOrder{
    "INSERT INTO order_processing.orders (order_nr, executor_id, assigned_at) "
    "VALUES ($1, $2, NOW()) "
    "ON CONFLICT DO NOTHING",
    userver::storages::postgres::Query::Name{"insert_order"},
};

const userver::storages::postgres::Query kSelectOrderByNrAndExecutor{
    "SELECT order_nr FROM order_processing.orders WHERE order_nr=$1 AND "
    "executor_id=$2",
    userver::storages::postgres::Query::Name{"select_order_by_nr_and_executor"},
};

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {
  userver::formats::json::ValueBuilder result;
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  if (json["order_nr"].IsMissing()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);

    result["message"] = "missing order_nr in body";
    return result.ExtractValue();
  }

  if (json["executor_id"].IsMissing()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);

    result["message"] = "missing executor_id in body";
    return result.ExtractValue();
  }

  std::string order_nr = json["order_nr"].As<std::string>();
  std::string executor_id = json["executor_id"].As<std::string>();

  userver::storages::postgres::Transaction transaction = pg_cluster_->Begin(
      "insert_order_trx", userver::storages::postgres::ClusterHostType::kMaster,
      {});

  auto res =
      transaction.Execute(kSelectOrderByNrAndExecutor, order_nr, executor_id);

  if (res.Size() != 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
    result["message"] = "order already exists";
    return result.ExtractValue();
  }

  res = transaction.Execute(kInsertOrder, order_nr, executor_id);
  transaction.Commit();

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  result["message"] = "success";

  return result.ExtractValue();
}

}  // namespace handlers::order::assign
