#include <Arduino.h>
#include <drivers/I2CManager.h>
#include "../include/ProjectConfig.h"
#include <Wire.h>

using namespace Drivers;

SemaphoreHandle_t I2CManager::_mutex = NULL;

void I2CManager::begin() {
  if (_mutex == NULL) {
    _mutex = xSemaphoreCreateMutex();
  }
  // Initialize Wire on default pins (SDA=21, SCL=22 on ESP32)
  // Si se necesitan pines específicos, agregar i2c_sda e i2c_scl a globalConfig
  Wire.begin();
}

SemaphoreHandle_t I2CManager::getMutex() {
  return _mutex;
}
