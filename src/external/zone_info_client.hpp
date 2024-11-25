#pragma once

#include <optional>
#include <string>
#include <userver/clients/http/client.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/component_context.hpp>

namespace external::zone_info {

/// Структура для представления информации о зоне
struct ZoneInfo {
    std::string id;      // Идентификатор зоны
    std::string name;    // Название зоны
    float coef;          // Коэффициент стоимости доставки

    static std::optional<ZoneInfo> ParseFromJson(const userver::formats::json::Value& json);
};

class ZoneInfoClient {
 public:
  explicit ZoneInfoClient(userver::clients::http::Client& http_client);

  std::optional<ZoneInfo> FetchZoneInfo(const std::string& zone_id) const;

 private:
  userver::clients::http::Client& http_client_;
};

}  // namespace external::zone_info