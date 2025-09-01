#ifndef __SERVO_CONFIG_H__
#define __SERVO_CONFIG_H__

/**
 * @file SERVO_CONFIG.h
 * @brief Configuración centralizada del sistema de riego multi-zona con servomotores.
 * 
 * Este archivo contiene todos los parámetros configurables del sistema de control
 * de servomotores para el riego por zonas. Modificar estos valores permite adaptar
 * el comportamiento del sistema sin necesidad de recompilar todo el código.
 * 
 * IMPORTANTE: Los valores aquí definidos afectan directamente el comportamiento
 * físico de los servomotores y el flujo de agua. Modificar con precaución.
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0
 * @date 2025
*/

// =============================================================================
// Includes necesarios
// =============================================================================
#include <stdint.h>
#include <Arduino.h>

// =============================================================================
// Configuración de Ángulos de Servomotores
// =============================================================================

// Ángulo de posición cerrada (válvula cerrada - sin flujo de agua)
// La mayoría de servos estándar usan 0° como posición inicial
constexpr uint8_t SERVO_CLOSED_ANGLE = 0;

// Ángulo de posición abierta (válvula abierta - flujo máximo de agua)
// Este valor puede necesitar ajuste según el tipo de válvula y servo usado
// Valores típicos: 90° para apertura completa, 45° para apertura parcial
constexpr uint8_t SERVO_OPEN_ANGLE = 90;

// =============================================================================
// Configuración de Tiempos de Riego
// =============================================================================

// Tiempo de riego por zona en segundos
// Este es el tiempo que cada zona recibirá agua de forma continua
// Ajustar según el tipo de plantas, tamaño de la zona y capacidad del sistema
constexpr uint32_t IRRIGATION_TIME_PER_ZONE_SECONDS = 300; // 5 minutos por zona

// Tiempo de transición entre zonas en segundos
// Pausa entre el cierre de una zona y la apertura de la siguiente
// Permite que la presión del sistema se estabilice y evita picos de consumo
constexpr uint32_t TRANSITION_TIME_SECONDS = 10; // 10 segundos de pausa

// Tiempo de apertura gradual del servo en milisegundos
// Control de la velocidad de apertura/cierre para evitar golpes de ariete
// Valores más altos = movimiento más suave pero más lento
constexpr uint32_t SERVO_MOVEMENT_TIME_MS = 1000; // 1 segundo para abrir/cerrar

// =============================================================================
// Configuración de PWM (Modulación por Ancho de Pulso)
// =============================================================================

// Frecuencia de la señal PWM para servomotores (Hz)
// Los servomotores estándar requieren 50Hz (período de 20ms)
constexpr uint32_t PWM_FREQUENCY = 50;

// Resolución del PWM en bits
// 12 bits proporciona 4096 pasos de resolución, suficiente para control preciso
constexpr uint8_t PWM_RESOLUTION = 12;

// Valores de pulso para control de servomotores
// Los servos estándar usan pulsos de 1-2ms dentro de un período de 20ms
// Para 50Hz y 12 bits de resolución (4096 pasos):
// - 1ms = 4096 * (1/20) = 204.8 ≈ 205 pasos
// - 2ms = 4096 * (2/20) = 409.6 ≈ 410 pasos
constexpr uint32_t PWM_MIN_PULSE = 205;  // ~1ms (0°)
constexpr uint32_t PWM_MAX_PULSE = 410;  // ~2ms (180°)

// =============================================================================
// Configuración de Seguridad y Límites
// =============================================================================

// Tiempo máximo de riego por zona (límite de seguridad)
// Evita el riego excesivo en caso de mal funcionamiento del sistema
constexpr uint32_t MAX_IRRIGATION_TIME_SECONDS = 1800; // 30 minutos máximo

// Tiempo mínimo de riego por zona
// Asegura que el riego sea efectivo y justifique el gasto energético
constexpr uint32_t MIN_IRRIGATION_TIME_SECONDS = 60; // 1 minuto mínimo

// Tiempo máximo de espera entre zonas
// Evita pausas excesivamente largas que podrían indicar un problema
constexpr uint32_t MAX_TRANSITION_TIME_SECONDS = 300; // 5 minutos máximo

// Número máximo de intentos de reposicionamiento de servo
// Si un servo no responde correctamente, se reintenta este número de veces
constexpr uint8_t MAX_SERVO_RETRY_ATTEMPTS = 3;

// =============================================================================
// Configuración de Zonas Específicas
// =============================================================================

// Estructura para configuración individual de zonas de servo
// Permite personalizar parámetros específicos para cada zona de riego
struct ServoZoneConfig {
    uint32_t irrigationTime;    // Tiempo de riego en segundos
    uint8_t openAngle;          // Ángulo de apertura personalizado
    bool enabled;               // Zona habilitada/deshabilitada
    const char* name;           // Nombre descriptivo de la zona
};

// Configuración predeterminada para todas las zonas
// Estos valores se pueden personalizar individualmente para cada zona
constexpr ServoZoneConfig DEFAULT_ZONE_CONFIG = {
    .irrigationTime = IRRIGATION_TIME_PER_ZONE_SECONDS,
    .openAngle = SERVO_OPEN_ANGLE,
    .enabled = true,
    .name = "Zona de Riego"
};

// Array de configuraciones específicas por zona
// Índice 0 = Zona 1, Índice 1 = Zona 2, etc.
// Permite personalización individual de cada zona del sistema
const ServoZoneConfig ZONE_CONFIGURATIONS[] = {
    {300, 90, true, "Jardín Frontal"},      // Zona 1: 5 min, apertura completa
    {240, 75, true, "Jardín Lateral"},      // Zona 2: 4 min, apertura parcial  
    {360, 90, true, "Huerta Trasera"},      // Zona 3: 6 min, apertura completa
    {180, 60, true, "Árboles Frutales"},    // Zona 4: 3 min, apertura reducida
    {420, 90, true, "Césped Principal"}     // Zona 5: 7 min, apertura completa
};

// =============================================================================
// Configuración de Monitoreo y Diagnóstico
// =============================================================================

// Intervalo de reporte de estado en milisegundos
// Frecuencia con la que el sistema reporta el estado de los servos
constexpr uint32_t STATUS_REPORT_INTERVAL_MS = 5000; // Cada 5 segundos

// Habilitar modo de diagnóstico detallado
// Cuando está habilitado, proporciona información adicional para debugging
constexpr bool ENABLE_VERBOSE_LOGGING = true;

// Habilitar verificación de posición de servomotores
// Algunos servos pueden proporcionar retroalimentación de posición
constexpr bool ENABLE_POSITION_FEEDBACK = false;

#endif // __SERVO_CONFIG_H__
