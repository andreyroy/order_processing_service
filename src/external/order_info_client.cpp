#include "order_info_client.hpp"
#include <userver/logging/log.hpp>
#include <userver/formats/json.hpp>

namespace external::order_info {

std::optional<OrderInfo> OrderInfo::ParseFromJson(const userver::formats::json::Value& json) {
    try {
        return OrderInfo{
            json["id"].As<std::string>(),
            json["zone_id"].As<std::string>(),
            json["customer_id"].As<std::string>(),
            json["base_cost"].As<double>()
        };
    } catch (const std::exception& ex) {
        LOG_ERROR() << "Failed to parse OrderInfo JSON: " << ex.what();
        return std::nullopt;
    }
}


OrderInfoClient::OrderInfoClient(userver::clients::http::Client& http_client)
    : http_client_(http_client) {}


std::optional<OrderInfo> OrderInfoClient::FetchOrderInfo(const std::string& order_id) const {
    const std::string url = "http://other-service/api/order-info?order_id=" + order_id;

    auto response = http_client_.CreateRequest()
                        .get(url)               // GET-запрос
                        .retry(2)               // Количество повторных попыток
                        .timeout(std::chrono::seconds(5)) // Таймаут
                        .perform();

    if (response->IsOk()) {
        try {
            auto json_response = userver::formats::json::FromString(response->body());
            return OrderInfo::ParseFromJson(json_response);
        } catch (const std::exception& ex) {
            // Логируем ошибку чтения JSON
            LOG_ERROR() << "Failed to parse response JSON from " << url << ": " << ex.what();
            return std::nullopt;
        }
    } else {
        auto status_code = response->status_code();
        if (status_code == 404) {
            LOG_WARNING() << "Order not found for ID: " << order_id;
        } else {
            LOG_ERROR() << "Failed to fetch order info from " << url
                        << ". HTTP code: " << static_cast<int>(status_code);
        }
        return std::nullopt;
    }
}

}  // namespace external::order_info