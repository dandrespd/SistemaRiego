#pragma once

#include "../../drivers/RTC_DS1302.h"
#include "../../drivers/Led.h"
#include "../../drivers/ServoPWMController.h"
#include "SystemManager.h"
#include "SystemConfig.h"

// Forward declarations to avoid circular dependencies
class RTC_DS1302;
class Led;
class ServoPWMController;
class SystemManager;

extern RTC_DS1302 rtcInstance;
extern Led statusLedInstance;
extern ServoPWMController servoControllerInstance;
extern SystemManager sistema;
extern SystemManager* sistemaRiego;
