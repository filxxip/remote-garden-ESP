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
    virtual void flushCache();

   protected:
    virtual float readValue() = 0;

   protected:
    std::shared_ptr<MqttClient> mqtt;
    SensorType sensorType;

   private:
    std::string topic;
    std::vector<float> cache;
};
