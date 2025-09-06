#include <esp_log.h>

#include <memory>
#include <vector>

#include "bluetooth_manager/bluetooth_manager.hpp"
#include "clients/actuators/factory/actuator_factory.hpp"
#include "clients/sensors/factory/sensor_factory.hpp"
#include "credentials_receiver/credentials_receiver.hpp"
#include "mqtt/handlers/device_control_handler/device_control_handler.hpp"
#include "mqtt/handlers/reset_handler/reset_handler.hpp"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"
#include "sensor_polling_manager/sensor_polling_manager.hpp"
#include "sensor_polling_manager/sensor_publisher/sensor_publisher.hpp"
#include "sensor_polling_manager/sensor_reader/sensor_reader.hpp"
#include "utils/device_provisioner/device_provisioner.hpp"
#include "utils/nvs_manager/nvs_manager.hpp"
#include "utils/spiffs_manager/spiffs_manager.hpp"
#include "wifi_manager/wifi_manager.hpp"

namespace {
constexpr const char* TAG = "Main";
}

extern "C" void app_main(void) {
    if (!SpiffsManager::init()) {
        ESP_LOGE(TAG, "Failed to initialize Spiffs!");
        return;
    }
    if (!NvsManager::init()) {
        ESP_LOGE(TAG, "Failed to initialize NVS!");
        return;
    }

    // @TODO remove scope
    // if (DeviceProvisioner::removeProvisioned()) {
    //     ESP_LOGI(TAG, "Provisioned files removed successfully");
    // } else {
    //     ESP_LOGW(TAG, "Some provisioned files could not be removed");
    // }

    auto wifi = std::make_shared<WifiManager>();
    wifi->init();

    auto mqtt = std::make_shared<MqttClient>();

    MockActuatorFactory actuatorFactory;
    MockSensorFactory sensorFactory;

    std::array<std::unique_ptr<SensorPollingManager>, 2> sensorManagers;

    mqtt->registerOnDisconnected([mqtt, &actuatorFactory, &sensorFactory,
                                  &sensorManagers, wifi](MqttClient& client) {
        // mqtt->unsubscribe("device/control");
        // mqtt->unsubscribe("reset");
        sensorManagers = {};
    });

    mqtt->registerOnConnected([mqtt, &actuatorFactory, &sensorFactory,
                               &sensorManagers, wifi](MqttClient& client) {
        auto atomize = actuatorFactory.create(ActuatorType::Atomize, mqtt);
        auto fan = actuatorFactory.create(ActuatorType::Fan, mqtt);
        auto heating = actuatorFactory.create(ActuatorType::HeatingMat, mqtt);
        auto water = actuatorFactory.create(ActuatorType::Water, mqtt);

        auto deviceHandler = std::make_shared<DeviceControlHandler>(
            std::move(atomize), std::move(fan), std::move(heating),
            std::move(water));
        auto resetHandler = std::make_shared<ResetHandler>(wifi, mqtt);

        mqtt->subscribe("device/control", std::move(deviceHandler));
        mqtt->subscribe("reset", std::move(resetHandler));

        std::vector<std::shared_ptr<SensorClient>> sensors = {
            sensorFactory.create(SensorType::Light, mqtt, wifi),
            sensorFactory.create(SensorType::AirHumidity, mqtt, wifi),
            sensorFactory.create(SensorType::SoilMoisture, mqtt, wifi),
            sensorFactory.create(SensorType::AirTemperature, mqtt, wifi),
            sensorFactory.create(SensorType::Battery, mqtt, wifi),
            sensorFactory.create(SensorType::SignalStrength, mqtt, wifi),
        };

        sensorManagers = {std::make_unique<SensorReader>(sensors, 3000),
                          std::make_unique<SensorPublisher>(sensors, 20000)};
        // sprawdzic co jak dwa rady sie connect na mqtt zrobi np po kolejny
        // bluetoth sieci
    });

    CredentialsReceiver credentialReceiver{std::make_unique<BluetoothManager>(),
                                           wifi, mqtt};
    credentialReceiver.init();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
