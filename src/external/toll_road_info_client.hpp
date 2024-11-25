#pragma once

#include <optional>
#include <string>
#include <userver/clients/http/client.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/component_context.hpp>

namespace external::toll_road_info {

struct TollRoadInfo {
    std::string id;       // Идентификатор платной дороги
    std::string zone_id;  // Идентификатор зоны
    float cost;           // Стоимость дороги

    static std::optional<TollRoadInfo> ParseFromJson(const userver::formats::json::Value& json);
};

class TollRoadInfoClient {
 public:
  explicit TollRoadInfoClient(userver::clients::http::Client& http_client);

  std::optional<TollRoadInfo> FetchTollRoadInfo(const std::string& zone_id) const;

 private:
  userver::clients::http::Client& http_client_;
};

}  // namespace external::toll_road_info
