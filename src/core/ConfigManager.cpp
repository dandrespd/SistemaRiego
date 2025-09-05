#include <Arduino.h>
#include <core/ConfigManager.h>
#include "../../include/core/SystemConfig.h"
#include "core/utils/Logger.h"
#include <SPIFFS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <MD5Builder.h>

ConfigManager* ConfigManager::instance = nullptr;

// ... existing implementation ...

bool ConfigManager::initialize() {
    if (!SPIFFS.begin(true)) {
        LOG_ERROR("[CONFIG] Failed to mount SPIFFS");
        return false;
    }

    _firstBoot = !SPIFFS.exists(configPath);

    if (_firstBoot) {
        LOG_INFO("[CONFIG] First boot detected");
        setDefaultConfiguration();
        if (!saveConfiguration()) {
            LOG_ERROR("[CONFIG] Failed to save initial configuration");
            return false;
        }
        LOG_INFO("[CONFIG] Initial configuration saved");
    } else {
        if (!loadConfiguration()) {
            LOG_ERROR("[CONFIG] Failed to load configuration");
            return false;
        }
        LOG_INFO("[CONFIG] Configuration loaded");
    }
    return true;
}

bool ConfigManager::isFirstBoot() const {
    return _firstBoot;
}

void ConfigManager::setFirstBoot(bool firstBoot) {
    _firstBoot = firstBoot;
    saveConfiguration(); // Persist the state
}

// ... rest of existing implementation ...
