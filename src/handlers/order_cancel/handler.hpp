#pragma once

#include <string>
#include <string_view>

#include <userver/components/component.hpp>
#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace handlers::order::cancel {

class Handler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-order-cancel";

  Handler(const userver::components::ComponentConfig& config,
          const userver::components::ComponentContext& component_context)
      : HttpHandlerJsonBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()) {}

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& json,
      userver::server::request::RequestContext&) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace handlers::order::cancel
