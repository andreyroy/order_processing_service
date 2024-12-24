#pragma once

#include <chrono>
#include <unordered_map>

#include <userver/components/component_base.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/formats/json.hpp>

#include "models/zone_info.hpp"

namespace caches::zones {

struct CachedZoneInfo {
  models::ZoneInfo zone_info;
  std::chrono::steady_clock::time_point updated_at;

  bool operator==(const CachedZoneInfo& other) const {
    return zone_info == other.zone_info;
  }
}

template <>
struct std::hash<Key> {
  std::size_t operator()(const CachedZoneInfo& zone_info) const {
    return ((std::hash<std::string>()(zone_info.id) ^
             (std::hash<std::string>()(zone_info.name) << 1)) >>
            1);
  }
};

class ZonesCache final : public components::ComponentBase {
 public:
  static constexpr std::string_view kName = "zones-cache";

  ZonesCache(const components::ComponentConfig& config,
             const components::ComponentContext& context) = default;
  ~ZonesCache() final = default;

  models::ZoneInfo GetZoneInfo(const std::string& zone_id) const;
  void UpdateZoneInfo(models::ZoneInfo zone_info);

 private:
  std::unordered_map<std::string, CachedZoneInfo> cache_;
  userver::dynamic_config::Source config_;
};

}  // namespace caches::zones