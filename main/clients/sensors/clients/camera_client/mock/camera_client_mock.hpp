#pragma once

#include <random>

#include "../camera_client.hpp"

class MockCameraSensorClient : public BaseCameraSensorClient {
   public:
    MockCameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                           const SensorType type);

   protected:
    bool captureImage(std::vector<uint8_t>& outImage) override;
    void cleanupCamera() override;

   private:
    std::mt19937 rng;
};