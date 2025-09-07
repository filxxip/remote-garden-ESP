#!/bin/bash

esptool.py --port /dev/ttyUSB0 --chip esp32 erase_flash --force