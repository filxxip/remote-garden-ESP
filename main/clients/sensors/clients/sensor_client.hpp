#pragma once

#include <memory>
#include <string>
#include <vector>

#include "clients/sensors/sensor_type.hpp"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"

class SensorClient {
   public:
    SensorClient(std::shared_ptr<MqttClient> mqttClient, const SensorType type);
    virtual ~SensorClient() = default;

    void storeReading();
    void flushCache();

   protected:
    virtual float readValue() = 0;

   protected:
    SensorType sensorType;

   private:
    std::shared_ptr<MqttClient> mqtt;
    std::string topic;
    std::vector<float> cache;
};
