#pragma once

#include <optional>
#include <string>

#include <userver/clients/http/client.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>

#include "models/zone_info.hpp"

namespace external::zone_info {

class ZoneInfoClient {
 public:
  explicit ZoneInfoClient(userver::clients::http::Client& http_client);

  std::optional<models::ZoneInfo> FetchZoneInfo(
      const std::string& zone_id) const;

 private:
  userver::clients::http::Client& http_client_;
};

}  // namespace external::zone_info