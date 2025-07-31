#pragma once

#include <string>

namespace DeviceProvisioner {

bool ensureProvisioning();
bool isProvisioned();

using TlsCred = std::tuple<std::string, std::string, std::string, std::string>;
TlsCred loadTlsCredentials();

}  // namespace DeviceProvisioner
