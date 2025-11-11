#pragma once

namespace Options {

/**
 * Get full MQTT connection string
 * @return Full MQTT URL with protocol and port (mqtts://server:8883)
 */
const char* getMqttFullUrl();

/**
 * Get API server URL based on build configuration
 * @return API server URL with protocol and path
 */
const char* getServerUrl();

/**
 * Check if running in production mode
 * @return true if production build, false if development
 */
bool isProduction();
}  // namespace Options
