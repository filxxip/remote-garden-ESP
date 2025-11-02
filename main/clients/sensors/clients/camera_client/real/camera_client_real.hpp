#pragma once

#include <vector>

#include "../camera_client.hpp"
#include "esp_camera.h"
#include "esp_log.h"

class CameraSensorClient : public BaseCameraSensorClient {
   public:
    explicit CameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                                SensorType type, size_t maxCacheSize = 10);

   protected:
    bool initializeCamera() override;
    void cleanupCamera() override;
    bool captureImage(std::vector<uint8_t>& outImage) override;
};
