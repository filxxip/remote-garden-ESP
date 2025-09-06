#pragma once

#include <memory>
#include <string>

#include "bluetooth_manager/bluetooth_manager.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"
#include "wifi_manager/wifi_manager.hpp"

class CredentialsReceiver {
   public:
    CredentialsReceiver(std::unique_ptr<BluetoothManager> newBle,
                        std::shared_ptr<WifiManager> newWifi,
                        std::shared_ptr<MqttClient> newMqtt);
    void init();

   protected:
    virtual void onCredentialsReceived(const std::string& ssid,
                                       const std::string& password,
                                       const std::string& userKey);

   private:
    void handleBleWrite(std::string& target, const uint8_t* data, uint16_t len);
    std::unique_ptr<BluetoothManager> ble;
    std::shared_ptr<WifiManager> wifi;
    std::shared_ptr<MqttClient> mqtt;

    bool mqttInitialized = false;
    std::string ssid;
    std::string password;
    std::string userKey;

    SemaphoreHandle_t mutex;
};
