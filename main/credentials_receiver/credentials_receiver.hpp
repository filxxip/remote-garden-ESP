#pragma once

#include <memory>
#include <string>

#include "bluetooth_manager/bluetooth_manager.hpp"
#include "wifi_manager/wifi_manager.hpp"

class CredentialsReceiver {
   public:
    CredentialsReceiver(std::unique_ptr<BluetoothManager> newBle,
                        std::shared_ptr<WifiManager> newWifi);
    void init();

   protected:
    virtual void onCredentialsReceived(const std::string& ssid,
                                       const std::string& password);

   private:
    std::unique_ptr<BluetoothManager> ble;
    std::shared_ptr<WifiManager> wifi;
    std::string ssid;
    std::string password;
};
