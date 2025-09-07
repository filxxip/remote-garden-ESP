#include "spiffs_polling_manager.hpp"

#include <esp_log.h>

#include "utils/spiffs_manager/spiffs_manager.hpp"

namespace {
constexpr const char* TAG = "SPIFFS_PM";

namespace SpiffsPaths {
constexpr const char* CA_CERT = ".ca.crt";
constexpr const char* DEVICE_SECRET = ".device_secret.txt";
}  // namespace SpiffsPaths

constexpr std::array<const char*, 2> SPIFFS_FILES = {
    SpiffsPaths::CA_CERT, SpiffsPaths::DEVICE_SECRET};

std::string stripDotPrefix(const char* path) {
    if (path[0] == '.') {
        return std::string(path + 1);
    }
    return std::string(path);
}
}  // namespace

void SpiffsPollingManager::moveFile(const char* path) const {
    std::string content;
    if (!SpiffsManager::readBlank(path, content)) {
        return;
    }

    std::string dest = stripDotPrefix(path);

    if (SpiffsManager::writeEncrypted(dest, content)) {
        SpiffsManager::deleteFile(path);
        ESP_LOGI(TAG, "Encrypted %s -> %s", path, dest.c_str());
    } else {
        ESP_LOGE(TAG, "Failed to encrypt file %s", path);
    }
}

void SpiffsPollingManager::pollAction() const {
    for (const auto* path : SPIFFS_FILES) {
        if (SpiffsManager::fileExists(path)) {
            moveFile(path);
        }
    }
}
