#ifndef __SERVO_MOTOR_H__
#define __SERVO_MOTOR_H__

/**
 * @file ServoMotor.h
 * @brief Clase base para control individual de servomotores con PWM nativo ESP32.
 * 
 * Esta clase encapsula la lógica básica de control de un servomotor individual,
 * proporcionando una base sólida para implementaciones más complejas como
 * sistemas multi-zona.
 * 
 * CARACTERÍSTICAS:
 * - Control PWM nativo del ESP32 (sin librerías externas)
 * - Movimiento suave con tiempos configurables
 * - Sistema de energización/desenergización inteligente
 * - Validación robusta de configuración
 * - Compatible con la interfaz ServoControllerInterface
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Clase Base Refactorizada
 * @date 2025
 */

#include <Arduino.h>
#include <stdint.h>
#include "ServoControllerInterface.h"
#include "SET_PIN.h"
#include "SERVO_CONFIG.h"

class ServoMotor : public ServoControllerInterface {
protected:
    int pinServo;                       // Pin GPIO del servomotor
    ServoControlState currentState;     // Estado actual del servo
    unsigned long stateStartTime;       // Tiempo de inicio del estado actual
    bool isInitialized;                 // Flag de inicialización
    bool autoCycle;                     // Ciclo automático habilitado
    unsigned long cycleCount;           // Contador de ciclos completados
    bool isEnergized;                   // Estado de energización
    
    // Configuración de ángulos (configurable)
    int angleClosed;
    int angleOpen;
    
    // Configuración de tiempos (configurable, en milisegundos)
    unsigned long movementDelay;
    unsigned long freeDelay;
    
    // Configuración PWM
    int pwmChannel;
    int pwmFrequency;
    int pwmResolution;
    
    // Límites de pulso PWM (en microsegundos)
    int minPulseWidth;
    int maxPulseWidth;

    /**
     * @brief Convierte ángulo a duty cycle PWM
     * @param angle Ángulo en grados (0-180)
     * @return Valor duty cycle calculado
     */
    virtual uint32_t angleToDutyCycle(int angle);
    
    /**
     * @brief Energiza el servo (envía señal PWM)
     */
    virtual void energize();
    
    /**
     * @brief Desenergiza el servo (duty cycle 0)
     */
    virtual void deenergize();
    
    /**
     * @brief Obtiene el ángulo actual basado en el estado
     * @return Ángulo actual en grados
     */
    virtual int getCurrentAngle() const;

public:
    /**
     * @brief Constructor con configuración personalizable
     * @param pin Pin GPIO del servo
     * @param closedAngle Ángulo para posición cerrada (default: 0°)
     * @param openAngle Ángulo para posición abierta (default: 85°)
     * @param moveDelay Tiempo de movimiento en ms (default: 500ms)
     * @param freeDelay Tiempo libre en ms (default: 3000ms)
     */
    ServoMotor(int pin, 
               int closedAngle = 0, 
               int openAngle = 85,
               unsigned long moveDelay = 500,
               unsigned long freeDelay = 3000);
    
    virtual ~ServoMotor() = default;
    
    // Implementación de ServoControllerInterface
    virtual bool init() override;
    virtual void startCycle(bool enableAutoCycle = false) override;
    virtual void stopCycle() override;
    virtual void update() override;
    virtual void openServo() override;
    virtual void closeServo() override;
    virtual ServoControlState getState() const override;
    virtual unsigned long getCycleCount() const override;
    virtual bool isReady() const override;
    virtual unsigned long getStateElapsedTime() const override;
    virtual bool isServoEnergized() const override;
    virtual void printStatus() const override;
    
    /**
     * @brief Configura los límites de pulso PWM personalizados
     * @param minPulse Pulso mínimo en microsegundos (default: 500μs)
     * @param maxPulse Pulso máximo en microsegundos (default: 2500μs)
     */
    virtual void setPulseLimits(int minPulse = 500, int maxPulse = 2500);
    
    /**
     * @brief Configura parámetros PWM avanzados
     * @param channel Canal PWM (default: 0)
     * @param frequency Frecuencia PWM en Hz (default: 50)
     * @param resolution Resolución en bits (default: 16)
     */
    virtual void setPWMConfig(int channel = 0, int frequency = 50, int resolution = 16);
    
    /**
     * @brief Valida la configuración actual del servo
     * @return true si la configuración es válida
     */
    virtual bool validateConfiguration() const;
};

#endif // __SERVO_MOTOR_H__
