#include "credentials_receiver.hpp"

#include <esp_log.h>

#include "utils/device_provisioner/device_provisioner.hpp"
#include "utils/file_manager/file_manager.hpp"

#define CHAR_UUID_SSID 0xFF01
#define CHAR_UUID_PASS 0xFF02
#define CHAR_UUID_USER_KEY 0xFF03

namespace {
constexpr const char* MQTT_URL = "mqtts://192.168.100.3:8883";
constexpr const char* TAG = "CredentialsReceiver";
constexpr const char* USER_KEY_PATH = "/spiffs/user.txt";

struct ProvisioningTaskArgs {
    CredentialsReceiver* self;
    std::string userKey;
};

}  // namespace

CredentialsReceiver::CredentialsReceiver(
    std::unique_ptr<BluetoothManager> newBle,
    std::shared_ptr<WifiManager> newWifi, std::shared_ptr<MqttClient> newMqtt)
    : ble(std::move(newBle)), wifi(newWifi), mqtt(newMqtt) {}

void CredentialsReceiver::init() {
    ble->addCharacteristic({.uuid16 = CHAR_UUID_SSID,
                            .bufferSize = 32,
                            .label = "SSID",
                            .onWrite = [this](const auto* data, auto len) {
                                handleBleWrite(ssid, data, len);
                            }});

    ble->addCharacteristic({.uuid16 = CHAR_UUID_PASS,
                            .bufferSize = 64,
                            .label = "Password",
                            .onWrite = [this](const auto* data, auto len) {
                                handleBleWrite(password, data, len);
                            }});

    ble->addCharacteristic({.uuid16 = CHAR_UUID_USER_KEY,
                            .bufferSize = 64,
                            .label = "User Key",
                            .onWrite = [this](const auto* data, auto len) {
                                handleBleWrite(userKey, data, len);
                            }});

    ble->init();
}

void CredentialsReceiver::onCredentialsReceived(const std::string& ssid,
                                                const std::string& password,
                                                const std::string& userKey) {
    if (FileManager::fileExists(USER_KEY_PATH)) {
        std::string content;
        if (FileManager::readFromFile(USER_KEY_PATH, content);
            content != userKey) {
            ESP_LOGW(TAG,
                     "User key is defined and is different than received one");
            return;
        }
    } else {
        FileManager::writeToFile(USER_KEY_PATH, userKey);
        ESP_LOGI(TAG, "User key is writted to file");
    }

    vTaskDelay(pdMS_TO_TICKS(600));
    mqtt->stop();
    vTaskDelay(pdMS_TO_TICKS(600));
    wifi->disconnect();
    vTaskDelay(pdMS_TO_TICKS(600));

    wifi->registerOnConnected([this, ssid, userKey] {
        ESP_LOGI(TAG,
                 "WiFi connected, starting provisioning in separate task...");

        xTaskCreate(
            [](void* param) {
                auto* args = static_cast<ProvisioningTaskArgs*>(param);
                CredentialsReceiver* self = args->self;
                const std::string userKey = args->userKey;
                delete args;

                ESP_LOGI(TAG, "Provisioning task started. Free heap: %zu bytes",
                         esp_get_free_heap_size());

                bool success = DeviceProvisioner::ensureProvisioning();

                if (!success) {
                    ESP_LOGE(TAG, "Provisioning failed after Wi-Fi connected.");
                    vTaskDelete(NULL);
                    return;
                }

                ESP_LOGI(TAG, "Provisioning successful, initializing MQTT...");

                if (!self->mqttInitialized) {
                    self->mqtt->init(MQTT_URL);
                    self->mqttInitialized = true;
                }
                self->mqtt->start();

                std::string payload = R"({"userKey":")" + userKey + R"("})";
                self->mqtt->publish("conn", payload);
                // TODO handle in backend and save in database as esp device
                // param i wtedy jak zapisze to gdy ktos bedzie chciec dorzucic
                // sobie jakies esp to mu wyswietli te z jego obecnej sieci i je
                // moze dodac do ogrodka wiec najpierw connect a potem dopiero
                // przeszukuje - najpiuerw podaje da esp dane sieci do jakiej ma
                // sie laczyc a potem moze dowolnie sobie laczyc je z apka i
                // garden, tylko ze statusem online pokaze albo jakis bool jaki
                // status

                ESP_LOGI(TAG, "Provisioning task completed successfully.");
                vTaskDelete(NULL);
            },
            "provisioning_task", 12288, new ProvisioningTaskArgs{this, ssid}, 5,
            NULL);
    });

    wifi->connect(ssid, password);
}

void CredentialsReceiver::handleBleWrite(std::string& target,
                                         const uint8_t* data, uint16_t len) {
    ESP_LOGI(TAG, "Received data from BLE");
    target.assign(reinterpret_cast<const char*>(data), len);

    if (!ssid.empty() && !password.empty() && !userKey.empty()) {
        onCredentialsReceived(ssid, password, userKey);
        ssid.clear();
        password.clear();
        userKey.clear();
    }
}