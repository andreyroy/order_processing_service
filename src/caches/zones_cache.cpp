#include "zones_cache.hpp"

namespace caches::zones {

models::ZoneInfo ZonesCache::GetZoneInfo(const std::string& zone_id) const {
  if (auto it = cache_.find(zone_id); it != cache_.end()) {
    LOG_INFO() << "Found zone info in cache";
    const auto& cached_zone = it->second;

    const std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();

    if (cached_zone.updated_at + std::chrono::minutes(10) < now) {
      LOG_INFO() << "Got old cached info for zone_id=" << zone_id
                 << ", use default zone";

      return models::ZoneInfo{
          .id = zone_id,
          .name = "default",
          .coef = 1,
      };
    }
  }

  LOG_INFO() << "Not found zone info for zone_id=" << zone_id
             << " in cache, use default zone";

  return models::ZoneInfo{
      .id = zone_id,
      .name = "default",
      .coef = 1,
  };
}

void ZonesCache::UpdateZoneInfo(models::ZoneInfo zone_info) {
  cache_[zone_info.id] = CachedZoneInfo{
      .zone_info = zone_info,
      .updated_at = std::chrono::steady_clock::now(),
  };
};

}  // namespace caches::zones