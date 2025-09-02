#ifndef __SET_PIN_H__
#define __SET_PIN_H__

/**
 * @file SET_PIN.h
 * @brief Definición centralizada de pines GPIO del proyecto de riego inteligente.
 * 
 * Este archivo centraliza todas las asignaciones de pines para facilitar
 * modificaciones futuras y evitar conflictos entre módulos del sistema.
 * 
 * IMPORTANTE: Antes de modificar cualquier pin, verificar que no esté siendo
 * usado por otro módulo y que sea compatible con la funcionalidad requerida
 * (PWM, ADC, I2C, SPI, etc.).
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0
 * @date 2025
*/

// =============================================================================
// Includes necesarios
// =============================================================================
#include <Arduino.h>
#include <stdint.h> // For uint8_t type

// =============================================================================
// Configuración de Pines por Módulos
// =============================================================================

// -------------------------------------------------------------------------
// Pines del RTC (DS1302) - Real Time Clock
// -------------------------------------------------------------------------
// Consolidated into SystemConfig.h - use HardwareConfig::RTC_RST etc.

// -------------------------------------------------------------------------
// LED de Estado del Sistema
// -------------------------------------------------------------------------
constexpr uint8_t LED = 16;         // LED externo para indicadores de estado
                                    // - Parpadeo lento: funcionamiento normal
                                    // - Parpadeo rápido: error del sistema
                                    // - Encendido fijo: modo configuración

// -------------------------------------------------------------------------
// Entradas y Salidas Digitales Básicas
// -------------------------------------------------------------------------
constexpr uint8_t IN_DIGITAL = 4;  // Entrada digital para sensor de humedad básico
// OUT_DIGITAL consolidated into SystemConfig.h - use HardwareConfig::OUT_DIGITAL

// -------------------------------------------------------------------------
// Pines de Servomotores PWM (Sistema de Riego Multi-Zona)
// -------------------------------------------------------------------------
// IMPORTANTE: Estos pines deben ser capaces de generar señales PWM
// Los pines GPIO válidos para PWM en ESP32 son: 0,1,2,3,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33
// 
// Cada servo controla una válvula de zona específica del sistema de riego:
// - Posición 0°: Válvula cerrada (sin flujo de agua)
// - Posición configurada: Válvula abierta (flujo de agua hacia la zona)
constexpr uint8_t SERVO_1_PIN = 25;  // Servo válvula zona 1 (jardín frontal)
constexpr uint8_t SERVO_2_PIN = 26;  // Servo válvula zona 2 (jardín lateral)
constexpr uint8_t SERVO_3_PIN = 27;  // Servo válvula zona 3 (huerta trasera)
constexpr uint8_t SERVO_4_PIN = 32;  // Servo válvula zona 4 (árboles frutales)
constexpr uint8_t SERVO_5_PIN = 33;  // Servo válvula zona 5 (césped principal)

// -------------------------------------------------------------------------
// Nuevo Controlador de Servo (Patrón Específico)
// -------------------------------------------------------------------------
// Pin para el nuevo controlador de servo que implementa el patrón específico
// proporcionado por el usuario usando la librería Servo.h
constexpr uint8_t SERVO_NUEVO_PIN = 9;  // Servo con patrón específico (0° ↔ 85°)

// Array de pines de servos para facilitar iteración programática
// Esto nos permite agregar o quitar servos fácilmente modificando solo este array
// y el sistema automáticamente se adapta al nuevo número de zonas
constexpr uint8_t SERVO_PINS[] = {
    SERVO_1_PIN,
    SERVO_2_PIN, 
    SERVO_3_PIN,
    SERVO_4_PIN,
    SERVO_5_PIN
};

// Número total de servos/zonas disponibles (calculado automáticamente)
// Este valor se actualiza automáticamente cuando se modifica el array SERVO_PINS
constexpr uint8_t NUM_SERVOS = sizeof(SERVO_PINS) / sizeof(SERVO_PINS[0]);

// -------------------------------------------------------------------------
// Pines Reservados para Expansiones Futuras
// -------------------------------------------------------------------------
// Los siguientes pines están disponibles para futuras expansiones:
// GPIO 0, 2, 4, 5, 12, 13, 14, 15, 22, 23
// 
// NOTA: Algunos pines tienen restricciones especiales:
// - GPIO 0: Usado durante el boot, requiere pull-up
// - GPIO 2: LED interno, usado durante el boot
// - GPIO 12: Afecta el voltaje flash durante el boot si está en HIGH
// - GPIO 15: Pull-up interno durante el boot
//
// Para sensores analógicos usar: GPIO 32, 33, 34, 35, 36, 37, 38, 39
// (GPIO 34-39 son solo de entrada y no tienen pull-up/pull-down internos)

#endif // __SET_PIN_H__
