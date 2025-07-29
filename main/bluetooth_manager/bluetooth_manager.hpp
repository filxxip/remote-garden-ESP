#pragma once

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>

#include <memory>
#include <string>
#include <vector>

#include "gatt_characteristic_def.hpp"

class BluetoothManager {
   public:
    BluetoothManager() = default;

    void addCharacteristic(GattCharacteristicDef&& def);
    void init();
    void enable();

   private:
    struct CharacteristicEntry {
        GattCharacteristicDef def;
        std::unique_ptr<uint8_t[]> buffer;
        uint16_t handle = 0;
    };

    esp_ble_adv_data_t advData{};
    esp_gatt_if_t gattIf = 0;
    uint16_t serviceHandle = 0;

    std::vector<CharacteristicEntry> characteristics;

    void registerServices();
    void handleGattsEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gattsIf,
                          esp_ble_gatts_cb_param_t* param);
    void handleGapEvent(esp_gap_ble_cb_event_t event,
                        esp_ble_gap_cb_param_t* param);

    void handleCreateService(const esp_ble_gatts_cb_param_t* param);
    void handleAddChar(const esp_ble_gatts_cb_param_t* param);
    void handleWriteEvent(const esp_ble_gatts_cb_param_t* param,
                          esp_gatt_if_t gattsIf);
    void handleDisconnectEvent();

    void registerCharacteristicInternal(CharacteristicEntry& entry);
    void initAdvData();
    void handleGapStartAdvertisingConfigure();
    void handleGapStartAdverisingStart(const esp_ble_gap_cb_param_t* param);
};
