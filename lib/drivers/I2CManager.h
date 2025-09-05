#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Drivers {
  class I2CManager {
  public:
    static void begin();
    static SemaphoreHandle_t getMutex();
  private:
    static SemaphoreHandle_t _mutex;
  };
}
