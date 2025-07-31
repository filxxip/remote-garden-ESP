#pragma once
#include <string>

namespace FileManager {

bool writeToFile(const std::string& path, const std::string& content);
bool readFromFile(const std::string& path, std::string& content);
bool fileExists(const std::string& path);

};  // namespace FileManager
