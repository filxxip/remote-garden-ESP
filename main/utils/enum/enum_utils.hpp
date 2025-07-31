#pragma once

#include <string>
#include <unordered_map>

template <typename EnumType>
std::unordered_map<std::string, EnumType> reverse_enum_map(
    const std::unordered_map<EnumType, std::string>& input) {
    std::unordered_map<std::string, EnumType> result;
    for (const auto& [enumVal, str] : input) {
        result[str] = enumVal;
    }
    return result;
}
