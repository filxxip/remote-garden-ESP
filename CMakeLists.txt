cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(esp32_dev_app)
# spiffs_create_partition_image(spiffs ${CMAKE_SOURCE_DIR}/spiffs FLASH_IN_PROJECT)