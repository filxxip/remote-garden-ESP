#pragma once

#include <memory>
#include <string>

#include "mqtt/mqtt_client/mqtt_client_impl.hpp"

class SensorClient {
   public:
    SensorClient(std::shared_ptr<MqttClient> mqttClient, std::string sensorName,
                 int gardenId);
    virtual ~SensorClient() = default;

    void publishReading();

   protected:
    virtual float readValue() = 0;

   private:
    std::shared_ptr<MqttClient> mqtt;
    std::string topic;
    std::string sensorName;
};
