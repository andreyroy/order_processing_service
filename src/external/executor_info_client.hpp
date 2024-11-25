#pragma once

#include <optional>
#include <string>
#include <vector>
#include <userver/clients/http/client.hpp>
#include <userver/formats/json.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/parse/common_containers.hpp>

namespace external::executor_info {

struct ExecutorInfo {
    std::string id;
    std::vector<std::string> tags;
    double rating;


    static std::optional<ExecutorInfo> ParseFromJson(const userver::formats::json::Value& json);
};

class ExecutorInfoClient {
 public:
  explicit ExecutorInfoClient(userver::clients::http::Client& http_client);

  std::optional<ExecutorInfo> FetchExecutorInfo(const std::string& executor_id) const;

 private:
  userver::clients::http::Client& http_client_;
};

}  // namespace external::executor_info