set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(
    SRCS
        "main.cpp"
        "wifi_manager/wifi_manager.cpp"
        "bluetooth_manager/bluetooth_manager.cpp"
        "nvs_manager/nvs_manager.cpp"
        "credentials_receiver/credentials_receiver.cpp"

        "mqtt/mqtt_client/mqtt_client_impl.cpp"
        "mqtt/my_handler/my_handler.cpp"
        "mqtt/handlers/garden_specifier_handler/garden_specifier_handler.cpp"
        "mqtt/handlers/device_control_handler/device_control_handler.cpp"

        "utils/json_utils/json_utils.cpp"

        "clients/actuator_client.cpp"
        "clients/water_client/mock/water_client_mock.cpp"
        "clients/water_client/real/water_client_real.cpp"
        "clients/atomize_client/mock/atomize_client_mock.cpp"
        "clients/atomize_client/real/atomize_client_real.cpp"
        "clients/fan_client/mock/fan_client_mock.cpp"
        "clients/fan_client/real/fan_client_real.cpp"
        "clients/heating_mat_client/mock/heating_mat_client_mock.cpp"
        "clients/heating_mat_client/real/heating_mat_client_real.cpp"

    INCLUDE_DIRS "." 
    REQUIRES bt nvs_flash mqtt json driver
)
