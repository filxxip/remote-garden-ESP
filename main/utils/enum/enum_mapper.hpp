#pragma once

#include <optional>
#include <string>
#include <unordered_map>

template <typename EnumType>
const std::unordered_map<EnumType, std::string>& getEnumToStrMap();

template <typename EnumType>
const std::unordered_map<std::string, EnumType>& getStrToEnumMap();

template <typename EnumType>
std::string enumToString(EnumType type) {
    const auto& map = getEnumToStrMap<EnumType>();
    auto it = map.find(type);
    return it != map.end() ? it->second : "unknown";
}

template <typename EnumType>
std::optional<EnumType> stringToEnum(const std::string& str) {
    const auto& map = getStrToEnumMap<EnumType>();
    auto it = map.find(str);
    return it != map.end() ? std::optional<EnumType>(it->second) : std::nullopt;
}
