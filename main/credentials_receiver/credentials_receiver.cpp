#include "credentials_receiver.hpp"

#include "mqtt/my_handler/my_handler.hpp"

#define CHAR_UUID_SSID 0xFF01
#define CHAR_UUID_PASS 0xFF02

namespace {
constexpr const char* MQTT_URL = "mqtt://192.168.100.3:1883";
}

CredentialsReceiver::CredentialsReceiver(
    std::unique_ptr<BluetoothManager> newBle,
    std::shared_ptr<WifiManager> newWifi, std::shared_ptr<MqttClient> newMqtt)
    : ble(std::move(newBle)), wifi(newWifi), mqtt(newMqtt) {}

void CredentialsReceiver::init() {
    ble->addCharacteristic(
        {.uuid16 = CHAR_UUID_SSID,
         .bufferSize = 32,
         .label = "SSID",
         .onWrite = [this](const uint8_t* data, uint16_t len) {
             ssid.assign(reinterpret_cast<const char*>(data), len);
             if (!password.empty()) {
                 onCredentialsReceived(ssid, password);
                 ssid.clear();
                 password.clear();
             }
         }});

    ble->addCharacteristic(
        {.uuid16 = CHAR_UUID_PASS,
         .bufferSize = 64,
         .label = "Password",
         .onWrite = [this](const uint8_t* data, uint16_t len) {
             password.assign(reinterpret_cast<const char*>(data), len);
             if (!ssid.empty()) {
                 onCredentialsReceived(ssid, password);
                 ssid.clear();
                 password.clear();
             }
         }});

    ble->init();
}

void CredentialsReceiver::onCredentialsReceived(const std::string& ssid,
                                                const std::string& password) {
    vTaskDelay(pdMS_TO_TICKS(600));
    mqtt->stop();
    vTaskDelay(pdMS_TO_TICKS(600));
    wifi->disconnect();
    vTaskDelay(pdMS_TO_TICKS(600));

    wifi->registerOnConnected([this] {
        if (!mqttInitialized) {
            mqtt->init(MQTT_URL);
        }
        mqtt->start();
    });
    wifi->connect(ssid, password);
}
