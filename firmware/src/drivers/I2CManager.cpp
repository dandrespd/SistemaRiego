#include "drivers/I2CManager.h"
#include "core/SystemConfig.h"

using namespace Drivers;

SemaphoreHandle_t I2CManager::_mutex = NULL;

void I2CManager::begin() {
  if (_mutex == NULL) {
    _mutex = xSemaphoreCreateMutex();
  }
  // Initialize Wire on configured pins
  Wire.begin(HardwareConfig::I2C_SDA, HardwareConfig::I2C_SCL);
}

SemaphoreHandle_t I2CManager::getMutex() {
  return _mutex;
}
