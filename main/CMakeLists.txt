idf_component_register(
    SRCS "main.cpp"
         "wifi_manager/wifi_manager.cpp"
         "bluetooth_manager/bluetooth_manager.cpp"
         "nvs_manager/nvs_manager.cpp"
         "credentials_receiver/credentials_receiver.cpp"
         "mqtt/mqtt_client/mqtt_client_impl.cpp"
    INCLUDE_DIRS "." "wifi_manager" "bluetooth_manager" "nvs_manager" "credentials_receiver" "mqtt" "mqtt/mqtt_client"
    REQUIRES bt nvs_flash mqtt
)
