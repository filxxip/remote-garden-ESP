#include "json_utils.hpp"

#include <esp_log.h>

static const char* TAG = "JSON_UTILS";

cJSON* JsonUtils::parseJson(const std::string& jsonStr) {
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        const char* errorPtr = cJSON_GetErrorPtr();
        if (errorPtr) {
            ESP_LOGI(TAG, "JSON parse error: %s\n", errorPtr);
        } else {
            ESP_LOGW(TAG, "Unknown JSON parse error\n");
        }
        return nullptr;
    }
    return root;
}
