#include <cstdio>

#include "bluetooth_manager/bluetooth_manager.hpp"
#include "credentials_receiver/credentials_receiver.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"
#include "nvs_manager/nvs_manager.hpp"
#include "wifi_manager/wifi_manager.hpp"

extern "C" void app_main(void) {
    NvsManager nvs;
    if (!nvs.init()) {
        printf("Failed to initialize NVS!\n");
        return;
    }

    auto wifi = std::make_shared<WifiManager>();
    wifi->init();
    auto mqtt = std::make_shared<MqttClient>();

    CredentialsReceiver credentialReceiver{std::make_unique<BluetoothManager>(),
                                           wifi, mqtt};
    credentialReceiver.init();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
