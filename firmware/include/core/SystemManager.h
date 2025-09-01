#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

/**
 * @file SystemManager.h
 * @brief Gestor principal del sistema de riego que coordina todos los módulos.
 * 
 * **CONCEPTO EDUCATIVO - PATRÓN FACADE + MEDIATOR**:
 * Esta clase actúa como un director de orquesta que coordina todos los módulos
 * del sistema sin que cada uno necesite conocer a los demás. Simplifica la
 * complejidad del main.cpp y mejora la mantenibilidad.
 * 
 * **BENEFICIOS DE ESTA REFACTORIZACIÓN**:
 * 1. main.cpp se enfoca solo en inicialización básica y loop
 * 2. Lógica de negocio encapsulada en una clase especializada
 * 3. Fácil testing y debugging de componentes individuales
 * 4. Escalabilidad mejorada para futuras funcionalidades
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.1 - Refactorización Arquitectónica
 * @date 2025
 */

#include "SystemConfig.h"
// Forward declarations to avoid circular includes
class RTC_DS1302;
class Led;
class ServoMotor;
class ServoPWMController;

// Forward declarations para evitar includes circulares
class WebSocketManager;

class SystemManager {
private:
    // Estados del sistema simplificados
    enum class SystemState {
        INITIALIZING,
        CONFIGURATION_MODE,  // Modo de configuración para RTC y otros ajustes
        NORMAL_OPERATION,
        ERROR_RECOVERY,
        EMERGENCY_STOP
    };

    // Referencias a módulos principales
    RTC_DS1302* rtc;
    Led* statusLed;
    ServoPWMController* servoController;
    WebSocketManager* wsManager;
    
    // Estado del sistema
    SystemState currentState;
    unsigned long lastStateChange;
    unsigned long lastMemoryCheck;
    unsigned long lastStatusReport;
    
    // Métricas del sistema
    uint32_t initialFreeMemory;
    uint32_t minimumFreeMemory;
    uint8_t consecutiveErrors;
    
    // Métodos privados para manejo de estados
    void handleInitializingState();
    void handleConfigurationMode();
    void handleNormalOperationState();
    void handleErrorRecoveryState();
    void handleEmergencyStopState();
    
    // Métodos privados de utilidad
    void monitorMemory();
    void generateStatusReport();
    bool validateSystemHealth();
    void updateStatusIndicators();

public:
    SystemManager(RTC_DS1302* rtc = nullptr, Led* statusLed = nullptr, ServoPWMController* servoController = nullptr);
    ~SystemManager();
    
    // Inicialización del sistema
    bool initialize();
    
    // Procesamiento principal (llamar desde loop)
    void update();
    
    // Control del sistema
    bool startIrrigationCycle();
    void stopIrrigationCycle();
    void emergencyStop();
    void resetSystem();
    
    // Estado del sistema
    bool isOperational() const;
    bool hasErrors() const;
    const char* getCurrentStateString() const;
    
    // Configuración
    void setWebSocketManager(WebSocketManager* wsManager);
    
    // Información del sistema
    void printSystemInfo() const;

    // Acceso a módulos para integración con Web (solo lectura)
    ServoPWMController* getIrrigationController() const { return servoController; }

    // Configuración del RTC desde web
    bool setRTCDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, uint8_t hour, uint8_t minute, uint8_t second);
};

#endif // __SYSTEM_MANAGER_H__
