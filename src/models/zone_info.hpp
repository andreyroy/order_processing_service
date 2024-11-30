#pragma once

#include <userver/formats/json.hpp>

namespace models {

/// Структура для представления информации о зоне
struct ZoneInfo {
  std::string id;    // Идентификатор зоны
  std::string name;  // Название зоны
  float coef;  // Коэффициент стоимости доставки

  static std::optional<ZoneInfo> ParseFromJson(
      const userver::formats::json::Value& json) {
    try {
      return ZoneInfo{
          json["id"].As<std::string>(),  // Получаем строку id зоны
          json["name"].As<std::string>(),  // Получаем строку имени зоны
          json["coef"].As<float>()  // Получаем коэффициент доставки (float)
      };
    } catch (const std::exception& ex) {
      LOG_ERROR() << "Failed to parse ZoneInfo JSON: " << ex.what();
      return std::nullopt;
    }
  };

  bool operator==(const ZoneInfo& other) const {
    return (id == other.id && name == other.name && coef == other.coef);
  }
};

}  // namespace models