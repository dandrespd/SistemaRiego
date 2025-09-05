/**
 * @file SystemManager.cpp
 * @brief Implementación del gestor principal del sistema de riego.
 * 
 * Esta implementación demuestra cómo convertir un main.cpp monolítico
 * en una arquitectura modular y mantenible.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Arduino.h>
#include <core/SystemManager.h>
#include "core/SystemConfig.h"
#include "core/EventBus.h"
#include "core/ConfigManager.h"
#include "drivers/I2CManager.h"
#include "drivers/RTC_DS1302.h"
#include "network/WiFiConfig.h"
#include "network/NTPTimeSync.h"
#include "utils/Logger.h"
#include "utils/SET_DATE.h"
#include "utils/GET_DATE.h"
#include "SystemManager.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "Utils.h"
#include "GET_DATE.h"
#include "SET_DATE.h"
#include "IN_DIGITAL.h"
#include "SET_PIN.h"
#include "WebSocketManager.h"
#include "SystemConfig.h"
#include "RTC_DS1302.h"
#include "Led.h"
#include "ServoPWMController.h"

SystemManager::SystemManager(RTC_DS1302* rtc, Led* statusLed, ServoPWMController* servoController)
    : rtc(rtc)
    , statusLed(statusLed)
    , servoController(servoController)
    , wsManager(nullptr)
    , currentState(SystemState::INITIALIZING)
    , lastStateChange(0)
    , lastMemoryCheck(0)
    , lastStatusReport(0)
    , initialFreeMemory(0)
    , minimumFreeMemory(0)
    , consecutiveErrors(0)
{
    initialFreeMemory = ESP.getFreeHeap();
    minimumFreeMemory = initialFreeMemory;
}

SystemManager::~SystemManager() {
    // Asegurar parada segura del sistema
    if (servoController) {
        servoController->closeServo();
    }
}

bool SystemManager::initialize() {
    LOG_INFO("Iniciando inicialización del sistema...");
    
    // **FASE 1: Inicializar ConfigManager**
    ConfigManager& config = ConfigManager::getInstance();
    if (!config.initialize()) {
        LOG_ERROR("Error inicializando configuración persistente");
        return false;
    }
    
    // **FASE 2: Validación de configuración**
    if (!SystemConfigValidator::validateAllConfiguration()) {
        LOG_ERROR("Configuración inválida - Abortando inicialización");
        return false;
    }
    
    // **FASE 3: Inicialización de módulos de hardware (inyectados)**
    if (rtc) {
        if (!rtc->init()) {
            LOG_ERROR("Error al inicializar RTC - Continuando en modo limitado");
            // No cambiar a ERROR_RECOVERY, permitir funcionamiento básico
        }
    } else {
        LOG_ERROR("RTC no inyectado - Funcionalidad de tiempo deshabilitada");
    }

    if (statusLed) {
        statusLed->init(LOW);
    } else {
        LOG_WARNING("LED de estado no inyectado");
    }
    
    // **FASE 4: Verificación y configuración del RTC**
    if (rtc) {
        // Intentar iniciar el RTC si está detenido
        if (rtc->isHalted()) {
            LOG_WARNING("[SystemManager] RTC detenido - Intentando iniciar automáticamente...");
            if (rtc->start()) {
                LOG_INFO("[SystemManager] RTC iniciado exitosamente");
            } else {
                LOG_ERROR("[SystemManager] No se pudo iniciar el RTC automáticamente");
            }
        }
        
        // Verificar si la fecha/hora es válida
        DateTime currentDateTime;
        if (rtc->getDateTime(&currentDateTime)) {
            if (!currentDateTime.isValid() || currentDateTime.year == 0) {
                LOG_WARNING("[SystemManager] RTC tiene fecha/hora inválida - Se requiere configuración");
                currentState = SystemState::ERROR_RECOVERY;
                lastStateChange = millis();
            }
        } else {
            LOG_ERROR("[SystemManager] No se pudo leer fecha/hora del RTC");
            currentState = SystemState::ERROR_RECOVERY;
            lastStateChange = millis();
        }
        
        // Si todavía está detenido después de intentar iniciar, requerir configuración
        if (rtc->isHalted()) {
            LOG_WARNING("[SystemManager] RTC todavía detenido - Se requiere configuración manual");
            LOG_INFO("[SystemManager] Ingrese la fecha y hora en formato: AAMMDDWHHMMSS");
            LOG_INFO("[SystemManager] Ejemplo: 2508306133200 para 30 de agosto de 2025, sábado, 13:32:00");
            currentState = SystemState::ERROR_RECOVERY;
            lastStateChange = millis();
        }
    } else {
        LOG_ERROR("No se puede verificar el RTC porque no fue inyectado");
        currentState = SystemState::ERROR_RECOVERY;
        lastStateChange = millis();
    }
    
    // **FASE 5: Inicialización del controlador de servos (inyectado)**
    if (servoController) {
        if (!servoController->init()) {
            LOG_ERROR("[SystemManager] Error al inicializar controlador de servos");
            currentState = SystemState::ERROR_RECOVERY;
            consecutiveErrors++;
            return false;
        }
    } else {
        LOG_ERROR("Controlador de servos no inyectado - Funcionalidad de riego deshabilitada");
        currentState = SystemState::EMERGENCY_STOP;
        return false;
    }
    
    // **FASE 6: Configuración de estado inicial**
    if (currentState != SystemState::ERROR_RECOVERY) {
        currentState = SystemState::NORMAL_OPERATION;
    }
    lastStateChange = millis();
    lastMemoryCheck = millis();
    lastStatusReport = millis();
    
    LOG_INFO("[SystemManager] Inicialización completada exitosamente");
    return true;
}

void SystemManager::update() {
    unsigned long currentTime = millis();
    
    // Handle WiFi reconnection (applies to all states)
    handleWiFiReconnection();
    
    // **ACTUALIZACIÓN PRINCIPAL SEGÚN ESTADO**
    switch (currentState) {
        case SystemState::INITIALIZING:
            handleInitializingState();
            break;
        case SystemState::CONFIGURATION_MODE:
            handleConfigurationMode();
            break;
        case SystemState::NORMAL_OPERATION:
            handleNormalOperationState();
            break;
        case SystemState::ERROR_RECOVERY:
            handleErrorRecoveryState();
            break;
        case SystemState::EMERGENCY_STOP:
            handleEmergencyStopState();
            break;
    }
    
    // **TAREAS PERIÓDICAS DEL SISTEMA**
    
    // Monitoreo de memoria cada 30 segundos
    if (currentTime - lastMemoryCheck >= 30000) {
        monitorMemory();
        lastMemoryCheck = currentTime;
    }
    
    // Reporte de estado cada 60 segundos
    if (currentTime - lastStatusReport >= 60000) {
        if (SystemDebug::ENABLE_VERBOSE_LOGGING) {
            generateStatusReport();
        }
        lastStatusReport = currentTime;
    }
    
    // **ACTUALIZACIÓN DE MÓDULOS PRINCIPALES**
    if (servoController) {
        servoController->update();
    }
    
    if (wsManager) {
        wsManager->update();
    }
    
    // Actualizar indicadores visuales
    updateStatusIndicators();
    
    // Validación de salud del sistema
    if (!validateSystemHealth()) {
        consecutiveErrors++;
        if (consecutiveErrors > SystemSafety::MAX_CONSECUTIVE_ERRORS) {
            currentState = SystemState::EMERGENCY_STOP;
            lastStateChange = currentTime;
        }
    } else {
        consecutiveErrors = 0;
    }
}

// WiFi reconnection with exponential backoff
void SystemManager::handleWiFiReconnection() {
    static unsigned long lastReconnectAttempt = 0;
    static int reconnectDelay = 1000; // Start with 1 second delay
    const int maxDelay = 60000; // Max 60 seconds delay
    
    if (WiFi.status() == WL_CONNECTED) {
        reconnectDelay = 1000; // Reset delay on successful connection
        return;
    }
    
    unsigned long currentTime = millis();
    
    if (currentTime - lastReconnectAttempt >= reconnectDelay) {
        DEBUG_PRINTLN("Attempting WiFi reconnection...");
        WiFi.reconnect();
        
        if (WiFi.waitForConnectResult() == WL_CONNECTED) {
            DEBUG_PRINTLN("WiFi reconnected successfully");
            reconnectDelay = 1000; // Reset delay
        } else {
            DEBUG_PRINTLN("WiFi reconnection failed, increasing delay");
            reconnectDelay = min(reconnectDelay * 2, maxDelay); // Exponential backoff
        }
        
        lastReconnectAttempt = currentTime;
    }
}

void SystemManager::handleConfigurationMode() {
    // En modo configuración, mostrar mensaje y esperar configuración del RTC
    static unsigned long lastConfigMessage = 0;
    unsigned long currentTime = millis();
    
    // Handle WiFi connection in configuration mode
    handleWiFiReconnection();
    
    if (currentTime - lastConfigMessage >= 5000) { // Mostrar mensaje cada 5 segundos
        LOG_INFO("\n🔧 🔧 🔧 MODO CONFIGURACIÓN ACTIVADO 🔧 🔧 🔧");
        LOG_WARNING("El sistema requiere configuración del RTC para operar");
        LOG_INFO("Opciones de configuración disponibles:");
        LOG_INFO("  1. Por puerto serial: Ingrese fecha/hora en formato AAMMDDWHHMMSS");
        LOG_INFO("  2. Por interfaz web: Acceda a http://" + WiFi.localIP().toString() + "/config");
        LOG_INFO("Ejemplo serial: 2508306141200 (30/08/2025, sábado, 14:12:00)");
        lastConfigMessage = currentTime;
    }
    
    // Intentar configuración automática desde serial
    if (rtc && rtc->isHalted()) {
        SetDate setDate(rtc);
        if (setDate.setDateFromSerial()) {
            LOG_INFO("[SystemManager] RTC configurado desde serial - Saliendo de modo configuración");
            currentState = SystemState::NORMAL_OPERATION;
            lastStateChange = millis();
        }
    }
}

void SystemManager::handleInitializingState() {
    // Este estado ahora es transitorio y se maneja directamente en initialize()
    // Si la inicialización es exitosa, el estado pasa a NORMAL_OPERATION o ERROR_RECOVERY
    // No debería permanecer en INITIALIZING
}

void SystemManager::handleNormalOperationState() {
    // En operación normal, mostrar fecha/hora cada segundo con protección contra loops
    static unsigned long lastDatePrint = 0;
    static unsigned long lastRtcCheck = 0;
    static int rtcErrorCount = 0;
    unsigned long currentTime = millis();
    
    // Verificar estado del RTC cada 5 segundos (no constantemente)
    if (currentTime - lastRtcCheck >= 5000) {
        if (rtc && rtc->isHalted()) {
            rtcErrorCount++;
            LOG_WARNING("[SystemManager] RTC detenido - Error #" + String(rtcErrorCount));
            
            // Si hay muchos errores consecutivos, entrar en modo recuperación
            if (rtcErrorCount > 5) {
                LOG_INFO("[SystemManager] Demasiados errores RTC - Entrando en modo recuperación");
                currentState = SystemState::ERROR_RECOVERY;
                lastStateChange = currentTime;
                rtcErrorCount = 0;
            }
        } else {
            rtcErrorCount = 0; // Reset counter if RTC is OK
        }
        lastRtcCheck = currentTime;
    }
    
    // Mostrar fecha/hora cada segundo solo si RTC está funcionando
    if (currentTime - lastDatePrint >= 1000) {
        if (rtc && !rtc->isHalted()) {
            GetDate fecha_hora(rtc);
            fecha_hora.printDate();
        }
        lastDatePrint = currentTime;
    }
}

void SystemManager::handleErrorRecoveryState() {
    // Verificar si hay datos disponibles en el puerto serial para configurar el RTC
    if (rtc && rtc->isHalted()) {
        // Intentar configuración automática del RTC
        SetDate setDate(rtc);
        if (setDate.setDateFromSerial()) {
            LOG_INFO("[SystemManager] RTC configurado - Recuperación exitosa");
            currentState = SystemState::NORMAL_OPERATION;
            lastStateChange = millis();
            consecutiveErrors = 0;
        }
    }
    
    // Intentar recuperación automática cada 10 segundos si no hay entrada serial
    static unsigned long lastRecoveryAttempt = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastRecoveryAttempt >= 10000) { // Intentar cada 10 segundos
        LOG_INFO("[SystemManager] Intentando recuperación automática...");
        
        // Podemos agregar aquí lógica adicional para recuperación automática
        // Por ahora, solo mostramos un mensaje recordando cómo configurar el RTC
        LOG_INFO("[SystemManager] Ingrese la fecha y hora en formato: AAMMDDWHHMMSS");
        LOG_INFO("[SystemManager] Ejemplo: 2512253143000 para 25 de diciembre de 2025, miércoles, 14:30:00");
        
        lastRecoveryAttempt = currentTime;
    }
}

void SystemManager::handleEmergencyStopState() {
    // En parada de emergencia, asegurar que todo esté detenido (solo una vez)
    static bool emergencyStopExecuted = false;
    if (servoController && !emergencyStopExecuted) {
        servoController->closeServo();
        emergencyStopExecuted = true;
        LOG_ERROR("[SystemManager] Parada de emergencia ejecutada - Válvulas cerradas");
    }
    
    // El LED debe parpadear muy rápido para indicar emergencia
    static unsigned long lastLedToggle = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastLedToggle >= 100) { // Parpadeo muy rápido
        statusLed->toggle();
        lastLedToggle = currentTime;
    }
    
    // Intentar recuperación automática después de 30 segundos en emergencia
    static unsigned long emergencyStartTime = millis();
    if (currentTime - emergencyStartTime >= 30000) { // 30 segundos
        LOG_INFO("[SystemManager] Intentando recuperación desde parada de emergencia...");
        resetSystem();
        emergencyStartTime = currentTime;
        emergencyStopExecuted = false;
    }
}

void SystemManager::monitorMemory() {
    uint32_t currentFreeMemory = ESP.getFreeHeap();
    
    if (currentFreeMemory < minimumFreeMemory) {
        minimumFreeMemory = currentFreeMemory;
    }
    
    const uint32_t CRITICAL_MEMORY_THRESHOLD = 10000; // 10KB
    
    if (currentFreeMemory < CRITICAL_MEMORY_THRESHOLD) {
        LOG_ERROR("[SystemManager] Memoria crítica: " + String(currentFreeMemory) + " bytes");
        consecutiveErrors++;
        
        if (currentFreeMemory < 5000) {
            LOG_ERROR("[SystemManager] Memoria extremadamente baja - Reiniciando sistema");
            ESP.restart();
        }
    }
}

void SystemManager::generateStatusReport() {
    LOG_DEBUG("\n" + repeatChar('=', 50));
    LOG_DEBUG("    REPORTE DE ESTADO DEL SISTEMA");
    LOG_DEBUG(repeatChar('=', 50));
    
    // Información del sistema
    LOG_DEBUG("🔧 Estado: " + String(getCurrentStateString()));
    LOG_DEBUG("💾 Memoria libre: " + String(ESP.getFreeHeap()) + " bytes");
    LOG_DEBUG("💾 Memoria mínima: " + String(minimumFreeMemory) + " bytes");
    LOG_DEBUG("⚠️ Errores consecutivos: " + String(consecutiveErrors));
    
    // Estado de módulos
    if (rtc) {
        LOG_DEBUG("⏰ RTC: " + String(rtc->isHalted() ? "DETENIDO" : "FUNCIONANDO"));
    }
    
    if (servoController) {
        LOG_DEBUG("🌱 Controlador servo: " + 
                     String(ServoControllerInterface::stateToString(servoController->getState())));
        LOG_DEBUG("🌱 Estado servo: " + String(ServoControllerInterface::stateToString(servoController->getState())));
    }
    
    LOG_DEBUG(repeatChar('=', 50) + "\n");
}

bool SystemManager::validateSystemHealth() {
    // Si el sistema ya está en parada de emergencia, recuperación de errores o modo configuración,
    // no realizar más validaciones que puedan incrementar errores
    if (currentState == SystemState::EMERGENCY_STOP || 
        currentState == SystemState::ERROR_RECOVERY ||
        currentState == SystemState::CONFIGURATION_MODE) {
        return true; // Evitar incrementar errores en estos estados
    }
    
    // Verificar salud básica del sistema
    bool healthy = true;
    
    // Verificar memoria disponible
    if (ESP.getFreeHeap() < 8000) {
        healthy = false;
        LOG_ERROR("[SystemManager] Memoria crítica detectada en validación de salud");
    }
    
    // Verificar estado del RTC - solo si estamos en operación normal
    // No tratar el RTC detenido como error fatal, solo como advertencia
    if (rtc && rtc->isHalted() && currentState == SystemState::NORMAL_OPERATION) {
        // En lugar de marcar como no saludable, entrar en modo configuración
        LOG_WARNING("[SystemManager] RTC detenido - Entrando en modo configuración");
        currentState = SystemState::CONFIGURATION_MODE;
        lastStateChange = millis();
        return true; // No incrementar errores por RTC
    }
    
    // Verificar errores en el controlador de servos
    // Note: ServoMotor doesn't have hasErrors() method, so we'll check state instead
    if (servoController && servoController->getState() == ServoControlState::ERROR) {
        healthy = false;
        LOG_ERROR("[SystemManager] Error en controlador de servos detectado");
    }
    
    return healthy;
}

void SystemManager::updateStatusIndicators() {
    if (!statusLed) return;
    
    static unsigned long lastLedUpdate = 0;
    unsigned long currentTime = millis();
    
    uint32_t blinkInterval;
    
    switch (currentState) {
        case SystemState::NORMAL_OPERATION:
            blinkInterval = 1000; // Parpadeo lento
            break;
        case SystemState::ERROR_RECOVERY:
            blinkInterval = 250;  // Parpadeo rápido
            break;
        case SystemState::EMERGENCY_STOP:
            blinkInterval = 100;  // Parpadeo muy rápido
            break;
        default:
            blinkInterval = 500;  // Parpadeo medio
            break;
    }
    
    if (currentTime - lastLedUpdate >= blinkInterval) {
        statusLed->toggle();
        lastLedUpdate = currentTime;
    }
}

// **MÉTODOS PÚBLICOS DE CONTROL**

bool SystemManager::startIrrigationCycle() {
    if (currentState != SystemState::NORMAL_OPERATION) {
        LOG_WARNING("[SystemManager] No se puede iniciar riego - Sistema no está en operación normal");
        return false;
    }
    
    if (servoController) {
        servoController->startCycle(true);
        return true;
    }
    
    return false;
}

void SystemManager::stopIrrigationCycle() {
    if (servoController) {
        servoController->stopCycle();
    }
}

void SystemManager::emergencyStop() {
    LOG_ERROR("[SystemManager] PARADA DE EMERGENCIA ACTIVADA");
    currentState = SystemState::EMERGENCY_STOP;
    lastStateChange = millis();
    
    if (servoController) {
        servoController->closeServo();
    }
}

void SystemManager::resetSystem() {
    LOG_INFO("[SystemManager] Reinicio del sistema solicitado");
    currentState = SystemState::INITIALIZING;
    lastStateChange = millis();
    consecutiveErrors = 0;
}

// **MÉTODOS DE CONSULTA**

bool SystemManager::isOperational() const {
    return currentState == SystemState::NORMAL_OPERATION;
}

bool SystemManager::hasErrors() const {
    return consecutiveErrors > 0 || currentState == SystemState::ERROR_RECOVERY;
}

const char* SystemManager::getCurrentStateString() const {
    switch (currentState) {
        case SystemState::INITIALIZING: return "INICIALIZANDO";
        case SystemState::NORMAL_OPERATION: return "OPERACIONAL";
        case SystemState::ERROR_RECOVERY: return "RECUPERANDO_ERRORES";
        case SystemState::EMERGENCY_STOP: return "PARADA_EMERGENCIA";
        default: return "DESCONOCIDO";
    }
}

void SystemManager::setWebSocketManager(WebSocketManager* manager) {
    wsManager = manager;
}

void SystemManager::printSystemInfo() const {
    // Print system information from SystemInfo namespace
    LOG_INFO("=== INFORMACIÓN DEL SISTEMA ===");
    LOG_INFO("Versión: " + String(SystemInfo::VERSION));
    LOG_INFO("Fecha de compilación: " + String(SystemInfo::BUILD_DATE) + " " + String(SystemInfo::BUILD_TIME));
    LOG_INFO("Compilador: " + String(SystemInfo::COMPILER_VERSION));
    LOG_INFO("Memoria libre: " + String(ESP.getFreeHeap()) + " bytes");
    LOG_INFO("Memoria mínima: " + String(minimumFreeMemory) + " bytes");
    LOG_INFO("Estado actual: " + String(getCurrentStateString()));
    LOG_INFO("Errores consecutivos: " + String(consecutiveErrors));
    LOG_INFO("===============================");
    
    SystemConfigValidator::printConfigurationSummary();
}

// Configuración del RTC desde web
bool SystemManager::setRTCDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, uint8_t hour, uint8_t minute, uint8_t second) {
    if (!rtc) {
        LOG_ERROR("[SystemManager] RTC no disponible para configuración");
        return false;
    }
    
    DateTime newDateTime(year, month, day, dayOfWeek, hour, minute, second);
    
    // Validar fecha/hora antes de configurar
    if (!newDateTime.isValid()) {
        LOG_ERROR("[SystemManager] Fecha/hora inválida para configuración");
        return false;
    }
    
    LOG_INFO("[SystemManager] Configurando RTC desde web: " + newDateTime.toString());
    
    if (rtc->setDateTime(newDateTime)) {
        LOG_INFO("[SystemManager] RTC configurado exitosamente desde web");
        
        // Si estábamos en modo configuración, salir a operación normal
        if (currentState == SystemState::CONFIGURATION_MODE) {
            LOG_INFO("[SystemManager] Saliendo de modo configuración - Sistema operacional");
            currentState = SystemState::NORMAL_OPERATION;
            lastStateChange = millis();
            consecutiveErrors = 0;
        }
        
        return true;
    } else {
        LOG_ERROR("[SystemManager] Error al configurar RTC desde web");
        return false;
    }
}
