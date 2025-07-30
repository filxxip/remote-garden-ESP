#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "cJSON.h"

namespace JsonUtils {

cJSON* parseJson(const std::string& jsonStr);

template <typename T>
static std::optional<T> getValue(cJSON* root,
                                 const std::vector<std::string>& path) {
    cJSON* current = root;
    for (const auto& key : path) {
        current = cJSON_GetObjectItemCaseSensitive(current, key.c_str());
        if (!current) {
            return std::nullopt;
        }
    }

    if constexpr (std::is_same<T, int>::value) {
        if (!cJSON_IsNumber(current)) {
            return std::nullopt;
        }
        return current->valueint;
    } else if constexpr (std::is_same<T, double>::value) {
        if (!cJSON_IsNumber(current)) {
            return std::nullopt;
        }
        return current->valuedouble;
    } else if constexpr (std::is_same<T, std::string>::value) {
        if (!cJSON_IsString(current) || !current->valuestring) {
            return std::nullopt;
        }
        return std::string(current->valuestring);
    } else if constexpr (std::is_same<T, bool>::value) {
        if (!cJSON_IsBool(current)) {
            return std::nullopt;
        }
        return cJSON_IsTrue(current);
    } else {
        static_assert(sizeof(T) == 0,
                      "Unsupported type in JsonUtils::getValue");
    }
}

template <typename T>
static std::optional<T> getValue(const std::string& jsonStr,
                                 const std::vector<std::string>& path) {
    cJSON* json = parseJson(jsonStr);
    if (!json) {
        return std::nullopt;
    }

    auto result = getValue<T>(json, path);
    cJSON_Delete(json);
    return result;
}

#define MAKE_JSON_GETTER(name, type)                                           \
    inline auto (*name)(const std::string&, const std::vector<std::string>&) = \
        JsonUtils::getValue<type>

MAKE_JSON_GETTER(getInt, int);
MAKE_JSON_GETTER(getString, std::string);
MAKE_JSON_GETTER(getDouble, double);
MAKE_JSON_GETTER(getBool, bool);
};  // namespace JsonUtils
