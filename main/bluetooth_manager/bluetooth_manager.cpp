#include "bluetooth_manager.hpp"

#include <esp_bt.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_log.h>

#include <cstring>

#define SERVICE_UUID 0x00FF
constexpr const char* TAG = "BLE";

void BluetoothManager::addCharacteristic(GattCharacteristicDef&& def) {
    CharacteristicEntry entry;
    entry.def = std::move(def);
    entry.buffer = std::make_unique<uint8_t[]>(entry.def.bufferSize);
    characteristics.push_back(std::move(entry));
}

void BluetoothManager::init() {
    static auto* instance = this;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_LOGI(TAG, "BLE init done");

    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(
        [](auto e, auto i, auto* p) { instance->handleGattsEvent(e, i, p); }));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(
        [](auto e, auto* p) { instance->handleGapEvent(e, p); }));

    ESP_ERROR_CHECK(esp_ble_gap_set_device_name("RemoteGardenESP32"));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));

    initAdvData();
    ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&advData));
}

void BluetoothManager::registerServices() {
    esp_gatt_srvc_id_t serviceId{};
    serviceId.is_primary = true;
    serviceId.id.inst_id = 0;
    serviceId.id.uuid.len = ESP_UUID_LEN_16;
    serviceId.id.uuid.uuid.uuid16 = SERVICE_UUID;

    ESP_ERROR_CHECK(esp_ble_gatts_create_service(gattIf, &serviceId, 10));
}

void BluetoothManager::handleCreateService(
    const esp_ble_gatts_cb_param_t* param) {
    serviceHandle = param->create.service_handle;
    ESP_LOGI(TAG, "Service created, handle: %d", serviceHandle);
    ESP_ERROR_CHECK(esp_ble_gatts_start_service(serviceHandle));

    for (auto& entry : characteristics) {
        registerCharacteristicInternal(entry);
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void BluetoothManager::registerCharacteristicInternal(
    CharacteristicEntry& entry) {
    esp_bt_uuid_t charUuid = {.len = ESP_UUID_LEN_16,
                              .uuid = {.uuid16 = entry.def.uuid16}};

    esp_attr_value_t attrVal{
        .attr_max_len = static_cast<uint16_t>(entry.def.bufferSize),
        .attr_len = 1,
        .attr_value = entry.buffer.get()};

    ESP_LOGI(TAG, "Registering %s...", entry.def.label);
    ESP_ERROR_CHECK(esp_ble_gatts_add_char(
        serviceHandle, &charUuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
        ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE, &attrVal,
        nullptr));
}

void BluetoothManager::handleAddChar(const esp_ble_gatts_cb_param_t* param) {
    uint16_t uuid = param->add_char.char_uuid.uuid.uuid16;
    uint16_t handle = param->add_char.attr_handle;

    for (auto& entry : characteristics) {
        if (entry.def.uuid16 == uuid) {
            entry.handle = handle;
            ESP_LOGI(TAG, "Characteristic 0x%04X (%s) registered at handle %d",
                     uuid, entry.def.label, handle);
            return;
        }
    }

    ESP_LOGW(TAG, "Unknown characteristic UUID 0x%04X", uuid);
}

void BluetoothManager::handleWriteEvent(const esp_ble_gatts_cb_param_t* param,
                                        esp_gatt_if_t gattsIf) {
    for (auto& entry : characteristics) {
        if (entry.handle == param->write.handle && entry.def.onWrite) {
            entry.def.onWrite(param->write.value, param->write.len);
        }
    }

    ESP_ERROR_CHECK(esp_ble_gatts_send_response(gattsIf, param->write.conn_id,
                                                param->write.trans_id,
                                                ESP_GATT_OK, nullptr));
}

void BluetoothManager::handleDisconnectEvent() {
    ESP_LOGI(TAG, "Received disconnected event");
    handleGapStartAdvertisingConfigure();  // to make ble available
                                           // again
}

void BluetoothManager::handleGattsEvent(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gattsIf,
                                        esp_ble_gatts_cb_param_t* param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            this->gattIf = gattsIf;
            registerServices();
            break;
        case ESP_GATTS_CREATE_EVT:
            handleCreateService(param);
            break;
        case ESP_GATTS_ADD_CHAR_EVT:
            handleAddChar(param);
            break;
        case ESP_GATTS_WRITE_EVT:
            handleWriteEvent(param, gattsIf);
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            handleDisconnectEvent();
            break;
        default:
            break;
    }
}

void BluetoothManager::handleGapEvent(esp_gap_ble_cb_event_t event,
                                      esp_ble_gap_cb_param_t* param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            handleGapStartAdvertisingConfigure();
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            handleGapStartAdverisingStart(param);
            break;
        default:
            break;
    }
}

void BluetoothManager::handleGapStartAdvertisingConfigure() {
    esp_ble_adv_params_t advParams{};
    advParams.adv_int_min = 0x00A0;
    advParams.adv_int_max = 0x00F0;
    advParams.adv_type = ADV_TYPE_IND;
    advParams.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    advParams.channel_map = ADV_CHNL_ALL;
    advParams.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

    ESP_ERROR_CHECK(esp_ble_gap_start_advertising(&advParams));
}

void BluetoothManager::handleGapStartAdverisingStart(
    const esp_ble_gap_cb_param_t* param) {
    if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
        ESP_LOGI(TAG, "Advertising started");
    } else {
        ESP_LOGE(TAG, "Advertising failed: %d", param->adv_start_cmpl.status);
    }
}

void BluetoothManager::initAdvData() {
    memset(&advData, 0, sizeof(advData));
    advData.set_scan_rsp = false;
    advData.include_name = true;
    advData.include_txpower = false;
    advData.min_interval = 0x20;
    advData.max_interval = 0x40;
    advData.appearance = 0x00;
    advData.flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT;
}
