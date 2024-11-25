#include "toll_road_info_client.hpp"
#include <userver/logging/log.hpp>
#include <userver/formats/json.hpp>

namespace external::toll_road_info {

std::optional<TollRoadInfo> TollRoadInfo::ParseFromJson(const userver::formats::json::Value& json) {
    try {
        return TollRoadInfo{
            json["id"].As<std::string>(),      // ID платной дороги
            json["zone_id"].As<std::string>(), // ID зоны
            json["cost"].As<float>()           // Стоимость
        };
    } catch (const std::exception& ex) {
        LOG_ERROR() << "Failed to parse TollRoadInfo JSON: " << ex.what();
        return std::nullopt;
    }
}


TollRoadInfoClient::TollRoadInfoClient(userver::clients::http::Client& http_client)
    : http_client_(http_client) {}

std::optional<TollRoadInfo> TollRoadInfoClient::FetchTollRoadInfo(const std::string& zone_id) const {

    const std::string url = "http://localhost:43101/api/toll-road-info?zone_id=" + zone_id;


    auto response = http_client_.CreateRequest()
                        .get(url)               // GET-запрос
                        .retry(2)               // Количество повторных попыток
                        .timeout(std::chrono::seconds(5)) // Таймаут
                        .perform();


    if (response->IsOk()) {
        try {
            auto json_response = userver::formats::json::FromString(response->body());
            return TollRoadInfo::ParseFromJson(json_response);
        } catch (const std::exception& ex) {
            LOG_ERROR() << "Failed to parse response JSON from " << url << ": " << ex.what();
            return std::nullopt;
        }
    } else {
        auto status_code = response->status_code();
        if (status_code == 404) {  // Код 404: Not Found
            LOG_WARNING() << "Toll road not found for zone ID: " << zone_id;
        } else {
            LOG_ERROR() << "Failed to fetch toll road info from " << url
                        << ". HTTP code: " << static_cast<int>(status_code);
        }
        return std::nullopt;
    }
}

}  // namespace external::toll_road_info