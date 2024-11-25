#include "zone_info_client.hpp"
#include <userver/logging/log.hpp>
#include <userver/formats/json.hpp>

namespace external::zone_info {

std::optional<ZoneInfo> ZoneInfo::ParseFromJson(const userver::formats::json::Value& json) {
    try {
        return ZoneInfo{
            json["id"].As<std::string>(),     // Получаем строку id зоны
            json["name"].As<std::string>(),   // Получаем строку имени зоны
            json["coef"].As<float>()          // Получаем коэффициент доставки (float)
        };
    } catch (const std::exception& ex) {
        LOG_ERROR() << "Failed to parse ZoneInfo JSON: " << ex.what();
        return std::nullopt;
    }
}

ZoneInfoClient::ZoneInfoClient(userver::clients::http::Client& http_client)
    : http_client_(http_client) {}

std::optional<ZoneInfo> ZoneInfoClient::FetchZoneInfo(const std::string& zone_id) const {
    const std::string url = "http://localhost:43101/api/zone-info?zone_id=" + zone_id;

    auto response = http_client_.CreateRequest()
                        .get(url)               // GET-запрос
                        .retry(2)               // Количество повторных попыток
                        .timeout(std::chrono::seconds(5)) // Таймаут
                        .perform();

    if (response->IsOk()) {
        try {
            auto json_response = userver::formats::json::FromString(response->body());
            return ZoneInfo::ParseFromJson(json_response);
        } catch (const std::exception& ex) {
            LOG_ERROR() << "Failed to parse response JSON from " << url << ": " << ex.what();
            return std::nullopt;
        }
    } else {
        auto status_code = response->status_code();
        if (status_code == 404) {  // Код 404: Not Found
            LOG_WARNING() << "Zone not found for ID: " << zone_id;
        } else {
            LOG_ERROR() << "Failed to fetch zone info from " << url
                        << ". HTTP code: " << static_cast<int>(status_code);
        }
        return std::nullopt;
    }
}

}  // namespace external::zone_info