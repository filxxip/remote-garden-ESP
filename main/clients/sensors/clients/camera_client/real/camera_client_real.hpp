#pragma once

#include "../camera_client.hpp"

class CameraSensorClient : public BaseCameraSensorClient {
   public:
    CameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                       const SensorType type);
    ~CameraSensorClient() override;

   protected:
    bool captureImage(std::vector<uint8_t>& outImage) override;
    bool initializeCamera() override;
    void cleanupCamera() override;
};