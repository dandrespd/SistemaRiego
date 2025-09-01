#ifndef __SERVO_CONFIG_H__
#define __SERVO_CONFIG_H__

/**
 * @file ServoConfig.h
 * @brief Configuración consolidada para el sistema de servomotores.
 * 
 * Este archivo centraliza todas las configuraciones relacionadas con servomotores
 * para eliminar redundancias y mantener consistencia en todo el sistema.
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Configuración Consolidada
 * @date 2025
 */

#include <stdint.h>

// =============================================================================
// Configuración General del Sistema de Servos
// =============================================================================

/**
 * @brief Configuración PWM estándar para servomotores
 */
constexpr int SERVO_PWM_FREQUENCY = 50;        // Frecuencia estándar para servos (50Hz)
constexpr int SERVO_PWM_RESOLUTION = 16;       // Resolución de 16 bits para precisión
constexpr int SERVO_PWM_CHANNEL_BASE = 0;      // Canal base para asignación

/**
 * @brief Límites de pulso PWM estándar para servomotores (en microsegundos)
 */
constexpr int SERVO_MIN_PULSE_WIDTH = 500;     // 0.5ms - posición mínima (0°)
constexpr int SERVO_MAX_PULSE_WIDTH = 2500;    // 2.5ms - posición máxima (180°)

/**
 * @brief Ángulos estándar para posiciones de servo
 */
constexpr int SERVO_ANGLE_CLOSED = 0;          // Posición cerrada (0°)
constexpr int SERVO_ANGLE_OPEN = 85;           // Posición abierta (85°)

/**
 * @brief Tiempos estándar para operación de servos (en milisegundos)
 */
constexpr unsigned long SERVO_MOVEMENT_DELAY = 500;     // Tiempo de movimiento (0.5s)
constexpr unsigned long SERVO_FREE_DELAY = 3000;        // Tiempo libre del servo (3s)

/**
 * @brief Configuración de reintentos y manejo de errores
 */
constexpr int MAX_SERVO_RETRY_ATTEMPTS = 3;    // Intentos máximos de reposicionamiento
constexpr unsigned long SERVO_ERROR_TIMEOUT = 10000;    // Timeout para errores (10s)

// =============================================================================
// Configuración de Zonas de Riego
// =============================================================================

/**
 * @brief Estructura de configuración para una zona de riego
 */
struct ServoZoneConfig {
    const char* name;                  // Nombre descriptivo de la zona
    uint8_t openAngle;                 // Ángulo para posición abierta
    uint32_t irrigationTime;           // Tiempo de riego en segundos
    uint8_t transitionDelay;           // Tiempo de transición entre zonas (segundos)
};

/**
 * @brief Configuración por defecto para zonas
 */
constexpr ServoZoneConfig DEFAULT_ZONE_CONFIG = {
    .name = "Zona Default",
    .openAngle = SERVO_ANGLE_OPEN,
    .irrigationTime = 300,             // 5 minutos por defecto
    .transitionDelay = 30              // 30 segundos entre zonas
};

/**
 * @brief Configuraciones específicas para cada zona (ejemplo)
 */
constexpr ServoZoneConfig ZONE_CONFIGURATIONS[] = {
    {"Zona 1 - Jardín Frontal", 85, 300, 30},
    {"Zona 2 - Jardín Trasero", 90, 450, 30},
    {"Zona 3 - Huerto", 80, 600, 45},
    {"Zona 4 - Macetas", 75, 180, 20}
};

// =============================================================================
// Configuración del Sistema de Riego Multi-Zona
// =============================================================================

/**
 * @brief Tiempos de operación del sistema de riego
 */
constexpr uint32_t MIN_IRRIGATION_TIME_SECONDS = 60;    // 1 minuto mínimo
constexpr uint32_t MAX_IRRIGATION_TIME_SECONDS = 3600;  // 1 hora máximo
constexpr uint32_t TRANSITION_TIME_SECONDS = 30;        // 30 segundos entre zonas

/**
 * @brief Configuración de reporting y logging
 */
constexpr unsigned long STATUS_REPORT_INTERVAL_MS = 60000;  // Reporte cada 60 segundos
constexpr bool ENABLE_VERBOSE_LOGGING = true;              // Logging detallado

// =============================================================================
// Funciones de Utilidad
// =============================================================================

/**
 * @brief Convierte microsegundos a duty cycle PWM
 * @param microseconds Microsegundos del pulso
 * @param frequency Frecuencia PWM en Hz
 * @param resolution Resolución PWM en bits
 * @return Duty cycle calculado
 */
inline uint32_t microsecondsToDutyCycle(uint32_t microseconds, uint32_t frequency, uint8_t resolution) {
    return (microseconds * frequency * (1UL << resolution)) / 1000000UL;
}

/**
 * @brief Convierte duty cycle a microsegundos
 * @param dutyCycle Duty cycle PWM
 * @param frequency Frecuencia PWM en Hz
 * @param resolution Resolución PWM en bits
 * @return Microsegundos calculados
 */
inline uint32_t dutyCycleToMicroseconds(uint32_t dutyCycle, uint32_t frequency, uint8_t resolution) {
    return (dutyCycle * 1000000UL) / (frequency * (1UL << resolution));
}

/**
 * @brief Valida configuración PWM
 * @param pin Pin del servo
 * @param frequency Frecuencia PWM
 * @param resolution Resolución PWM
 * @return true si la configuración es válida
 */
inline bool validatePWMConfiguration(int pin, int frequency, int resolution) {
    return (pin >= 0 && pin <= 39) &&
           (frequency >= 1 && frequency <= 40000) &&
           (resolution >= 1 && resolution <= 16);
}

#endif // __SERVO_CONFIG_H__
