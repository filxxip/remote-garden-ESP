#pragma once

#include <functional>
#include <string>

#include "esp_event_base.h"

class WifiManager {
   public:
    WifiManager() = default;
    void init();
    bool connect(const std::string& ssid, const std::string& password,
                 const int max_retries = 10);
    void disconnect();
    float getRssi() const;
    static void onGotIp(void* arg, esp_event_base_t base, int32_t id,
                        void* data);
    void registerOnConnected(std::function<void()> callback);
    std::string getBssid() const;

   private:
    std::function<void()> onConnectedCallback;
    bool apReceived = false;
};
