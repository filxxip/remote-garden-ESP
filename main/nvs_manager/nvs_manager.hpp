#pragma once

#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#include <string>

class NvsManager {
   public:
    bool init();
    bool writeString(const std::string& namespaceName, const std::string& key,
                     const std::string& value);
    bool readString(const std::string& namespaceName, const std::string& key,
                    std::string& outValue);
    bool writeInt(const std::string& namespaceName, const std::string& key,
                  int32_t value);
    bool readInt(const std::string& namespaceName, const std::string& key,
                 int32_t& outValue);

   private:
    const char* TAG = "NvsManager";
};
