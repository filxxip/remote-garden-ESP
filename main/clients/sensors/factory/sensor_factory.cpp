#include "sensor_factory.hpp"

#include <unordered_map>

#include "clients/sensors/clients/air_humidity_client/mock/air_humidity_client_mock.hpp"
#include "clients/sensors/clients/air_humidity_client/real/air_humidity_client_real.hpp"
#include "clients/sensors/clients/camera_client/mock/camera_client_mock.hpp"
#include "clients/sensors/clients/camera_client/real/camera_client_real.hpp"
#include "clients/sensors/clients/light_client/mock/light_sensor_client_mock.hpp"
#include "clients/sensors/clients/light_client/real/light_sensor_client_real.hpp"
#include "clients/sensors/clients/signal_strength_client/mock/signal_strength_client_mock.hpp"
#include "clients/sensors/clients/signal_strength_client/real/signal_strength_client_real.hpp"
#include "clients/sensors/clients/soil_moisture_client/mock/soil_moisture_client_mock.hpp"
#include "clients/sensors/clients/soil_moisture_client/real/soil_moisture_client_real.hpp"
#include "clients/sensors/clients/thermometer_client/mock/thermometer_client_mock.hpp"
#include "clients/sensors/clients/thermometer_client/real/thermometer_client_real.hpp"
#include "clients/sensors/clients/water_level_client/mock/water_level_client_mock.hpp"
#include "clients/sensors/clients/water_level_client/real/water_level_client_real.hpp"
#include "hal/gpio_types.h"

std::shared_ptr<SensorClient> MockSensorFactory::create(
    const SensorType type, std::shared_ptr<MqttClient> mqtt,
    std::shared_ptr<Dht11Reader> dhtReader,
    std::shared_ptr<WifiManager> wifi) const {
    switch (type) {
        case SensorType::AirHumidity:
            return std::make_shared<AirHumidityClientMock>(std::move(mqtt));
        case SensorType::Light:
            return std::make_shared<LightSensorClientMock>(std::move(mqtt));
        case SensorType::SoilMoisture:
            return std::make_shared<SoilMoistureClientMock>(std::move(mqtt));
        case SensorType::AirTemperature:
            return std::make_shared<ThermometerClientMock>(std::move(mqtt));
        case SensorType::WaterLevel:
            return std::make_shared<WaterLevelClientMock>(std::move(mqtt));
        case SensorType::SignalStrength:
            return std::make_shared<SignalStrengthClientMock>(std::move(mqtt));
        case SensorType::Camera:
            return std::make_shared<MockCameraSensorClient>(std::move(mqtt),
                                                            type);
        default:
            return nullptr;
    }
}

std::shared_ptr<SensorClient> AdcSensorFactory::create(
    const SensorType type, std::shared_ptr<MqttClient> mqtt,
    std::shared_ptr<Dht11Reader> dhtReader,
    std::shared_ptr<WifiManager> wifi) const {
    switch (type) {
        case SensorType::Camera:
            return std::make_shared<CameraSensorClient>(std::move(mqtt), type);

        case SensorType::AirTemperature:
            return std::make_shared<ThermometerClientReal>(
                std::move(mqtt), std::move(dhtReader));

        case SensorType::AirHumidity:
            return std::make_shared<AirHumidityClientReal>(
                std::move(mqtt), std::move(dhtReader));

        case SensorType::Light:
            return std::make_shared<LightSensorClientReal>(
                std::move(mqtt), I2C_NUM_1, (gpio_num_t)1, (gpio_num_t)2);

        case SensorType::SoilMoisture:
            return std::make_shared<SoilMoistureClientReal>(std::move(mqtt),
                                                            ADC1_CHANNEL_6);

        case SensorType::WaterLevel:
            return std::make_shared<WaterLevelClientReal>(std::move(mqtt),
                                                          I2C_NUM_1);

        default:
            return nullptr;
    }
}