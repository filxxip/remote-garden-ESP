#pragma once

#include <string>

class WifiManager {
   public:
    WifiManager() = default;
    void init() const;
    bool connect(const std::string& ssid, const std::string& password,
                 int max_retries = 10);
    bool isConnected() const;

   private:
    bool connected = false;
};
