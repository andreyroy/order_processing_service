#include "handler.hpp"

#include <userver/server/http/http_status.hpp>
#include <userver/storages/query.hpp>

namespace handlers::order::cancel {

const userver::storages::postgres::Query kSelectOrderByNrAndExecutor{
    "SELECT cancelled_at FROM order_processing.orders WHERE order_nr=$1 AND "
    "executor_id=$2 and acquired_at IS NULL",
    userver::storages::postgres::Query::Name{"select_order_by_nr_and_executor"},
};

const userver::storages::postgres::Query kCancelOrder{
    "UPDATE order_processing.orders SET cancel_reason = $3, cancelled_at = "
    "NOW() "
    "WHERE order_nr=$1 "
    "AND executor_id=$2",
    userver::storages::postgres::Query::Name{"select_order_by_nr_and_executor"},
};

userver::formats::json::Value Handler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {
  userver::formats::json::ValueBuilder result;
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  if (json["order_nr"].IsMissing() || json["executor_id"].IsMissing() ||
      json["reason"].IsMissing()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);

    result["message"] = "missing required parameters";
    return result.ExtractValue();
  }

  std::string order_nr = json["order_nr"].As<std::string>();
  std::string executor_id = json["executor_id"].As<std::string>();
  std::string reason = json["reason"].As<std::string>();

  userver::storages::postgres::Transaction transaction = pg_cluster_->Begin(
      "cancel_order_trx", userver::storages::postgres::ClusterHostType::kMaster,
      {});

  auto res =
      transaction.Execute(kSelectOrderByNrAndExecutor, order_nr, executor_id);

  if (res.Size() == 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    result["message"] = "order not found";
    return result.ExtractValue();
  }

  auto canceled_at = res.AsSingleRow<
      std::optional<userver::storages::postgres::TimePointTz>>();

  if (canceled_at.has_value()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    result["message"] = "order already canceled";
    return result.ExtractValue();
  }

  res = transaction.Execute(kCancelOrder, order_nr, executor_id, reason);
  transaction.Commit();

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  result["message"] = "success";

  return result.ExtractValue();
}

}  // namespace handlers::order::cancel
