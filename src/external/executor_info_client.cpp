#include "executor_info_client.hpp"
#include <userver/logging/log.hpp>
#include <userver/formats/json.hpp>

namespace external::executor_info {

std::optional<ExecutorInfo> ExecutorInfo::ParseFromJson(const userver::formats::json::Value& json) {
    try {
        return ExecutorInfo{
            json["id"].As<std::string>(),
            json["tags"].As<std::vector<std::string>>(),
            json["rating"].As<double>()
        };
    } catch (const std::exception& ex) {
        LOG_ERROR() << "Failed to parse ExecutorInfo JSON: " << ex.what();
        return std::nullopt;
    }
}

ExecutorInfoClient::ExecutorInfoClient(userver::clients::http::Client& http_client)
    : http_client_(http_client) {}


std::optional<ExecutorInfo> ExecutorInfoClient::FetchExecutorInfo(const std::string& executor_id) const {

    const std::string url = "http://localhost:43101/api/executor-info?executor_id=" + executor_id;

    auto response = http_client_.CreateRequest()
                        .get(url)               // GET-запрос
                        .retry(2)               // Количество повторных попыток
                        .timeout(std::chrono::seconds(5)) // Таймаут
                        .perform();


    if (response->IsOk()) {
        try {

            auto json_response = userver::formats::json::FromString(response->body());
            return ExecutorInfo::ParseFromJson(json_response);
        } catch (const std::exception& ex) {

            LOG_ERROR() << "Failed to parse response JSON from " << url << ": " << ex.what();
            return std::nullopt;
        }
    } else {

        auto status_code = response->status_code();
        if (status_code == 404) {  // Код 404: Not Found
            LOG_WARNING() << "Executor not found for ID: " << executor_id;
        } else {
            LOG_ERROR() << "Failed to fetch executor info from " << url
                        << ". HTTP code: " << static_cast<int>(status_code);
        }
        return std::nullopt;
    }
}

}  // namespace external::executor_info