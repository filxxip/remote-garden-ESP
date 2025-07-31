#pragma once

#include <esp_log.h>

#include <string>

namespace NvsManager {

bool init();
bool writeString(const std::string& ns, const std::string& key,
                 const std::string& value);
bool readString(const std::string& ns, const std::string& key,
                std::string& outValue);
bool writeInt(const std::string& ns, const std::string& key, int32_t value);
bool readInt(const std::string& ns, const std::string& key, int32_t& outValue);

}  // namespace NvsManager
