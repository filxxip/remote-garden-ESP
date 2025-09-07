set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(
    SRCS
        main.cpp

        bluetooth_manager/bluetooth_manager.cpp

        clients/actuators/actuator_type.cpp
        clients/actuators/clients/actuator_client.cpp
        clients/actuators/clients/gpio_actuator_client.cpp
        clients/actuators/clients/mock_actuator_client.cpp
        clients/actuators/factory/actuator_factory.cpp

        clients/sensors/clients/adc_sensor_client.cpp
        clients/sensors/clients/sensor_client.cpp

        clients/sensors/clients/air_humidity_client/mock/air_humidity_client_mock.cpp
        clients/sensors/clients/air_humidity_client/real/air_humidity_client_real.cpp

        clients/sensors/clients/battery_client/mock/battery_sensor_client_mock.cpp
        clients/sensors/clients/battery_client/real/battery_sensor_client_real.cpp

        clients/sensors/clients/light_client/mock/light_sensor_client_mock.cpp
        clients/sensors/clients/light_client/real/light_sensor_client_real.cpp

        clients/sensors/clients/signal_strength_client/mock/signal_strength_client_mock.cpp
        clients/sensors/clients/signal_strength_client/real/signal_strength_client_real.cpp

        clients/sensors/clients/soil_moisture_client/mock/soil_moisture_client_mock.cpp
        clients/sensors/clients/soil_moisture_client/real/soil_moisture_client_real.cpp

        clients/sensors/clients/thermometer_client/mock/thermometer_client_mock.cpp
        clients/sensors/clients/thermometer_client/real/thermometer_client_real.cpp

        clients/sensors/factory/sensor_factory.cpp
        clients/sensors/sensor_type.cpp

        credentials_receiver/credentials_receiver.cpp

        mqtt/handlers/device_control_handler/device_control_handler.cpp
        mqtt/handlers/reset_handler/reset_handler.cpp
        mqtt/handlers/stop_handler/stop_handler.cpp
        mqtt/handlers/resume_handler/resume_handler.cpp
        mqtt/mqtt_client/mqtt_client_impl.cpp

        polling_manager/polling_manager.cpp
        polling_manager/sensor_polling_manager/sensor_polling_manager.cpp
        polling_manager/sensor_polling_manager/sensor_publisher/sensor_publisher.cpp
        polling_manager/sensor_polling_manager/sensor_reader/sensor_reader.cpp
        polling_manager/spiffs_polling_manager/spiffs_polling_manager.cpp

        utils/crypto/crypto.cpp
        utils/device_provisioner/device_provisioner.cpp
        utils/file_manager/file_manager.cpp
        utils/json_utils/json_utils.cpp
        utils/nvs_manager/nvs_manager.cpp
        utils/nvs_manager/nvs_credentials_manager.cpp
        utils/http_manager/http_manager.cpp
        utils/spiffs_manager/spiffs_manager.cpp
        utils/scoped_lock/scoped_lock.cpp

        wifi_manager/wifi_manager.cpp

    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash mqtt json driver esp_adc spiffs mbedtls esp_http_client
)
