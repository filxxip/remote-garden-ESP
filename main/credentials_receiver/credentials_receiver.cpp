#include "credentials_receiver.hpp"

#define CHAR_UUID_SSID 0xFF01
#define CHAR_UUID_PASS 0xFF02

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
    if (wifi->connect(ssid, password)) {
        mqtt->init("mqtt://192.168.100.12:1883");
    }
}
