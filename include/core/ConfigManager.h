#pragma once

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

class ConfigManager {
private:
    static ConfigManager* instance;
    bool _firstBoot;
    ConfigManager();

public:
    static ConfigManager* getInstance();
    bool initialize();
    void setDefaultConfiguration();
    bool loadConfiguration();
    bool saveConfiguration();
    bool isFirstBoot() const;
    void setFirstBoot(bool firstBoot);
    
    // RTC configuration methods
    bool setRTCTime(const String& datetime);
    String getLastRTCTime();
    
    // Configuration value methods
    bool updateConfigValue(const String& key, const String& value);
    String getConfigValue(const String& key);
};

#endif // CONFIG_MANAGER_H
