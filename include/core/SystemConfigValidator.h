#pragma once

#ifndef SYSTEM_CONFIG_VALIDATOR_H
#define SYSTEM_CONFIG_VALIDATOR_H

#include <Arduino.h>

class SystemConfigValidator {
public:
    static bool validateSafetyLimits();
    static bool validateNetworkSettings();
    static bool validateHardwareConfiguration();
    static bool validateAllConfiguration();
    static void printConfigurationSummary();
};

#endif // SYSTEM_CONFIG_VALIDATOR_H
