#pragma once

#include <string>

namespace SpiffsManager {

bool init();
void deinit();

bool writeEncrypted(const std::string& path, const std::string& content);
bool readEncrypted(const std::string& path, std::string& content);
bool readBlank(const std::string& path, std::string& content);
bool fileExists(const std::string& path);
bool deleteFile(const std::string& path);

}  // namespace SpiffsManager
