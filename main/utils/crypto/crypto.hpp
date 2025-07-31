#pragma once
#include <string>

namespace Crypto {

bool generateRsaKeyAndCsr(const std::string& deviceId,
                          std::string& outPrivateKeyPem,
                          std::string& outCsrPem);

}