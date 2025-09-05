#pragma once

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>

// Forward declarations
class RTC_DS1302;
class Led;
class ServoPWMController;
class WebSocketManager;

enum class SystemState {
    INITIALIZING,
    CONFIGURATION_MODE,
    NORMAL_OPERATION,
    ERROR_RECOVERY,
    EMERGENCY_STOP
};

class SystemManager {
private:
    RTC_DS1302* rtc;
    Led* statusLed;
    ServoPWMController* servoController;
    WebSocketManager* wsManager;
    SystemState currentState;
    unsigned long lastStateChange;
    unsigned long lastMemoryCheck;
    unsigned long lastStatusReport;
    uint32_t initialFreeMemory;
    uint32_t minimumFreeMemory;
    int consecutiveErrors;

    void handleWiFiReconnection();
    void handleInitializingState();
    void handleConfigurationMode();
    void handleNormalOperationState();
    void handleErrorRecoveryState();
    void handleEmergencyStopState();
    void monitorMemory();
    void generateStatusReport();
    bool validateSystemHealth();
    void updateStatusIndicators();

public:
    SystemManager(RTC_DS1302* rtc, Led* statusLed, ServoPWMController* servoController);
    ~SystemManager();
    
    bool initialize();
    void update();
    
    // Control methods
    bool startIrrigationCycle();
    void stopIrrigationCycle();
    void emergencyStop();
    void resetSystem();
    
    // Query methods
    bool isOperational() const;
    bool hasErrors() const;
    const char* getCurrentStateString() const;
    
    void setWebSocketManager(WebSocketManager* manager);
    void printSystemInfo() const;
    
    // RTC configuration from web
    bool setRTCDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, uint8_t hour, uint8_t minute, uint8_t second);
};

#endif // SYSTEM_MANAGER_H
