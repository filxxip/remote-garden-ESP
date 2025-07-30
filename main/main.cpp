#include <esp_log.h>

#include <cstdio>

#include "bluetooth_manager/bluetooth_manager.hpp"
#include "clients/atomize_client/mock/atomize_client_mock.hpp"
#include "clients/fan_client/mock/fan_client_mock.hpp"
#include "clients/heating_mat_client/mock/heating_mat_client_mock.hpp"
#include "clients/water_client/mock/water_client_mock.hpp"
#include "credentials_receiver/credentials_receiver.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt/handlers/device_control_handler/device_control_handler.hpp"
#include "mqtt/handlers/garden_specifier_handler/garden_specifier_handler.hpp"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"
#include "mqtt/my_handler/my_handler.hpp"
#include "nvs_manager/nvs_manager.hpp"
#include "wifi_manager/wifi_manager.hpp"

static const char* TAG = "Main";

extern "C" void app_main(void) {
    NvsManager nvs;
    if (!nvs.init()) {
        ESP_LOGE(TAG, "Failed to initialize NVS!");
        return;
    }

    auto wifi = std::make_shared<WifiManager>();
    wifi->init();
    auto mqtt = std::make_shared<MqttClient>();
    auto gardenHandler = std::make_shared<GardenSpecifierHandler>();
    auto myHandler = std::make_shared<MyHandler>();

    gardenHandler->setCallback([mqtt](const auto& prev, auto updated) {
        ESP_LOGI(TAG, "Garden callback triggered");

        if (prev.has_value()) {
            ESP_LOGI(TAG, "Unsubscribing from previous garden ID: %d",
                     prev.value());
            mqtt->unsubscribe("device/" + std::to_string(prev.value()) +
                              "/control");
        }

        auto deviceHandler = std::make_shared<DeviceControlHandler>(
            std::make_unique<AtomizeClientMock>(mqtt, updated),
            std::make_unique<FanClientMock>(mqtt, updated),
            std::make_unique<HeatingMatClientMock>(mqtt, updated),
            std::make_unique<WaterClientMock>(mqtt, updated));
        mqtt->subscribe("device/" + std::to_string(updated) + "/control",
                        std::move(deviceHandler));
    });

    mqtt->registerOnConnected([gardenHandler](MqttClient& client) {
        client.subscribe("garden", std::move(gardenHandler));
    });

    CredentialsReceiver credentialReceiver{std::make_unique<BluetoothManager>(),
                                           wifi, mqtt};
    credentialReceiver.init();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
