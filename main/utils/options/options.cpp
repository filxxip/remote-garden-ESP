#include "options.hpp"

#include <stdio.h>

// URL jest definiowane przez CMake
#ifndef URL
#define URL "localhost"
#endif

namespace Options {

const char* getMqttFullUrl() {
    static char full_mqtt_url[128];

#ifdef MQTT_PRODUCTION
    snprintf(full_mqtt_url, sizeof(full_mqtt_url), "mqtts://%s:8883", URL);
#else
    snprintf(full_mqtt_url, sizeof(full_mqtt_url), "mqtts://%s:8883", URL);
#endif

    return full_mqtt_url;
}

const char* getServerUrl() {
    static char server_url[128];

#ifdef MQTT_PRODUCTION
    snprintf(server_url, sizeof(server_url), "https://%s/api", URL);
#else
    snprintf(server_url, sizeof(server_url), "https://%s/api", URL);
#endif

    return server_url;
}

bool isProduction() {
#ifdef MQTT_PRODUCTION
    return true;
#else
    return false;
#endif
}
}  // namespace Options