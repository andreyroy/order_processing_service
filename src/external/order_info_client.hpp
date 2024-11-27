#pragma once

#include <optional>
#include <string>
#include <userver/clients/http/client.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/component_context.hpp>

namespace external::order_info {

struct OrderInfo {
    std::string id;
    std::string zone_id;
    std::string customer_id;
    double base_cost;

    static std::optional<OrderInfo> ParseFromJson(const userver::formats::json::Value& json);
};

class OrderInfoClient {
 public:
  explicit OrderInfoClient(userver::clients::http::Client& http_client);

  std::optional<OrderInfo> FetchOrderInfo(const std::string& order_id) const;

 private:
  userver::clients::http::Client& http_client_;
};

}  // namespace external::order_info