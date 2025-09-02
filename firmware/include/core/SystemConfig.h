#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

/**
 * @file SystemConfig.h
 * @brief Configuración centralizada y consolidada para todo el sistema de riego.
 * 
 * **PRINCIPIO DE RESPONSABILIDAD ÚNICA**:
 * Tener un lugar central para toda la configuración del sistema reduce errores,
 * facilita el mantenimiento y mejora la legibilidad.
 * 
 * **BENEFICIOS DE ESTA ARQUITECTURA**:
 * 1. Configuración centralizada evita inconsistencias
 * 2. Fácil modificación de parámetros sin buscar en múltiples archivos
 * 3. Validación de configuración en un solo lugar
 * 4. Documentación consolidada de todos los parámetros del sistema
 * 
 * @author Sistema de Riego Inteligente  
 * @version 3.1 - Arquitectura mejorada con principios SOLID
 * @date 2025
 */

// =============================================================================
// Includes fundamentales
// =============================================================================

#include <stdint.h> // Para tipos enteros estándar
#include <Arduino.h>
#include "Utils.h"

// =============================================================================
// CONFIGURACIÓN GENERAL DEL SISTEMA
// =============================================================================

/**
 * @brief Configuración de debugging y monitoreo del sistema.
 */
namespace SystemDebug {
    constexpr bool ENABLE_SERIAL_DEBUGGING = true;      // Habilitar mensajes serie
    constexpr bool ENABLE_VERBOSE_LOGGING = true;       // Logging detallado
    constexpr bool ENABLE_PERFORMANCE_MONITORING = false; // Monitoreo de rendimiento
    constexpr bool ENABLE_MEMORY_MONITORING = false;    // Monitoreo de memoria
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;      // Velocidad comunicación serie
}

/**
 * @brief Configuración de seguridad y límites críticos del sistema.
 */
namespace SystemSafety {
    constexpr uint32_t MAX_TOTAL_IRRIGATION_TIME_MINUTES = 180;  // 3 horas máximo total
    constexpr uint32_t MIN_PRESSURE_STABILIZATION_TIME_MS = 2000; // Estabilización presión
    constexpr uint32_t WATCHDOG_TIMEOUT_MS = 30000;             // Timeout watchdog 30s
    constexpr uint8_t MAX_CONSECUTIVE_ERRORS = 5;               // Errores antes de parada
    constexpr uint32_t EMERGENCY_STOP_TIMEOUT_MS = 1000;       // Timeout parada emergencia
}

/**
 * @brief Configuración de red y conectividad.
 */
namespace NetworkConfig {
    constexpr uint32_t WIFI_CONNECTION_TIMEOUT_MS = 30000;     // 30s para conectar WiFi
    constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 5000;         // Reintentar cada 5s
    constexpr uint8_t MAX_WIFI_RETRY_ATTEMPTS = 10;           // Máximo 10 reintentos
    constexpr uint16_t WEB_SERVER_PORT = 80;                  // Puerto servidor web
    constexpr uint32_t WEB_REQUEST_TIMEOUT_MS = 5000;         // Timeout peticiones web
    constexpr uint32_t NTP_UPDATE_INTERVAL_MS = 3600000;      // Actualizar NTP cada 1 hora
    constexpr uint32_t NTP_TIMEOUT_MS = 5000;                 // Timeout para NTP
}

namespace RTCConfig {
    constexpr bool AUTO_SYNC_WITH_NTP = true;                // Sincronizar automáticamente con NTP
    constexpr const char* NTP_SERVER = "pool.ntp.org";        // Servidor NTP por defecto
    constexpr int TIMEZONE_OFFSET = -5;                      // Offset de zona horaria (UTC-5 para Bogotá)
    constexpr bool DAYLIGHT_SAVING = false;                   // No usar horario de verano
}

/**
 * @brief Configuración de seguridad y autenticación.
 */
namespace SecurityConfig {
    constexpr bool ENABLE_WEB_AUTHENTICATION = true;         // Habilitar autenticación web
    // Removed default credentials to force change on first boot
    // Temporary credentials generated on first boot
    static char* DEFAULT_WEB_USERNAME = nullptr;
    static char* DEFAULT_WEB_PASSWORD = nullptr;
    constexpr const char* OTA_PASSWORD = "riego2025secure";   // Contraseña para actualizaciones OTA
    constexpr uint32_t SESSION_TIMEOUT_MS = 3600000;         // Timeout de sesión 1 hora
    constexpr uint8_t MAX_LOGIN_ATTEMPTS = 5;               // Máximo intentos de login
    constexpr uint32_t LOGIN_COOLDOWN_MS = 300000;          // Enfriamiento después de intentos fallidos (5 minutos)
}

// =============================================================================
// CONFIGURACIÓN DE HARDWARE - PINES Y PERIFÉRICOS
// =============================================================================

/**
 * @brief Configuración de pines GPIO para el sistema de riego.
 */
namespace HardwareConfig {
    // Pines para RTC DS1302 (consolidated from SET_PIN.h)
    constexpr uint8_t RTC_RST = 25;      // Pin RST (Reset) del módulo RTC DS1302
    constexpr uint8_t RTC_SCLK = 26;     // Pin SCLK (Serial Clock) del módulo RTC
    constexpr uint8_t RTC_IO = 32;       // Pin IO (Input/Output Data) del módulo RTC
    
    // Pines para servomotores (válvulas de riego)
    constexpr uint8_t SERVO_PINS[] = {13, 12, 14, 33, 15};  // Eliminado pin 27, agregado 15
    constexpr uint8_t NUM_SERVOS = 5;
    constexpr uint8_t SERVO_PIN = SERVO_PINS[0]; // Pin por defecto para servo principal
    
    // Pin para LED de estado
    constexpr uint8_t LED = 2;
    
    // Pin para sensor de humedad (entrada digital)
    constexpr uint8_t IN_DIGITAL = 34;
    
    // Pines para salidas digitales adicionales (consolidated from SET_PIN.h)
    constexpr uint8_t OUT_DIGITAL = 4;   // Salida digital para control de válvula principal
}

/**
 * @brief Configuración específica de servomotores.
 */
namespace ServoConfig {
    constexpr uint32_t PWM_FREQUENCY = 50;           // 50 Hz para servos estándar
    constexpr uint8_t PWM_RESOLUTION = 12;           // 12 bits de resolución (4096 pasos)
    constexpr uint32_t PWM_MIN_PULSE = 205;          // 1ms para 0° (4096 * 1/20)
    constexpr uint32_t PWM_MAX_PULSE = 410;          // 2ms para 180° (4096 * 2/20)
    constexpr uint8_t SERVO_CLOSED_ANGLE = 0;        // Ángulo para válvula cerrada
    constexpr uint32_t SERVO_MOVEMENT_TIME_MS = 1000; // Tiempo movimiento servo
    constexpr uint8_t MAX_SERVO_RETRY_ATTEMPTS = 3;  // Máximo intentos de reposicionamiento
}

/**
 * @brief Configuración de zonas de riego por defecto.
 */
namespace ZoneTimingConfig {
    constexpr uint32_t MIN_IRRIGATION_TIME_SECONDS = 60;    // Mínimo 60 segundos por zona
    constexpr uint32_t MAX_IRRIGATION_TIME_SECONDS = 1800;  // Máximo 30 minutos por zona (unificado de ServoConfig.h)
    constexpr uint32_t TRANSITION_TIME_SECONDS = 10;       // 10 segundos entre zonas (consistente con SERVO_CONFIG)
    constexpr uint32_t STATUS_REPORT_INTERVAL_MS = 5000;  // Reporte cada 5 segundos
}

// =============================================================================
// VALIDACIÓN AUTOMÁTICA DE CONFIGURACIÓN
// =============================================================================

/**
 * @class SystemConfigValidator
 * @brief Clase utilitaria para validar la configuración del sistema.
 * 
 * Nota: La implementación completa se encuentra en SystemConfigValidator.cpp
 * para evitar problemas de inclusión y dependencias.
 */
class SystemConfigValidator {
public:
    /**
     * @brief Valida toda la configuración del sistema.
     * @return true si toda la configuración es válida, false en caso contrario
     */
    static bool validateAllConfiguration();
    
    /**
     * @brief Imprime un resumen completo de la configuración actual.
     */
    static void printConfigurationSummary();

private:
    static bool validateSafetyLimits();
    static bool validateNetworkSettings();
    static bool validateHardwareConfiguration();
};

// =============================================================================
// MACROS DE UTILIDAD PARA DEBUGGING
// =============================================================================

#include <stdio.h>

#define DEBUG_PRINT(x) do { if (SystemDebug::ENABLE_SERIAL_DEBUGGING) { printf("%s", String(x).c_str()); } } while(0)
#define DEBUG_PRINTLN(x) do { if (SystemDebug::ENABLE_SERIAL_DEBUGGING) { printf("%s\n", String(x).c_str()); } } while(0)
#define DEBUG_PRINTF(format, ...) do { if (SystemDebug::ENABLE_SERIAL_DEBUGGING) { printf(format, ##__VA_ARGS__); } } while(0)
#define VERBOSE_PRINT(x) do { if (SystemDebug::ENABLE_VERBOSE_LOGGING) { printf("%s", String(String("[VERBOSE] ") + String(x)).c_str()); } } while(0)
#define VERBOSE_PRINTLN(x) do { if (SystemDebug::ENABLE_VERBOSE_LOGGING) { printf("%s\n", String(String("[VERBOSE] ") + String(x)).c_str()); } } while(0)

// =============================================================================
// INFORMACIÓN DE VERSION Y BUILD
// =============================================================================

namespace SystemInfo {
    constexpr const char* VERSION = "3.2.0";  // Updated version to reflect enhancements
    constexpr const char* BUILD_DATE = __DATE__;
    constexpr const char* BUILD_TIME = __TIME__;
    constexpr const char* COMPILER_VERSION = __VERSION__;
}

#endif // __SYSTEM_CONFIG_H__
