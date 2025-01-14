#pragma once

#include <string>
#include <string_view>

#include <userver/components/component.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json.hpp>

namespace handlers::order::acquire {

class Handler final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-order-acquire";

  Handler(const userver::components::ComponentConfig& config,
          const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()),
        http_client_(
            component_context
                .FindComponent<userver::components::HttpClient>().GetHttpClient()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
  userver::clients::http::Client& http_client_;
};

}  // namespace handlers::order::acquire