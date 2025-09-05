/**
 * @file ServoMotor.cpp
 * @brief Implementación de la clase base para control individual de servomotores.
 * 
 * Esta clase proporciona la funcionalidad básica para controlar un servomotor
 * individual usando PWM nativo del ESP32. Sirve como base para implementaciones
 * más complejas como sistemas multi-zona.
 * 
 * CARACTERÍSTICAS:
 * - Control PWM nativo del ESP32 (LEDC)
 * - Movimiento suave con tiempos configurables
 * - Sistema de energización/desenergización inteligente
 * - Validación robusta de configuración
 * - Compatible con ServoControllerInterface
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Clase Base Refactorizada
 * @date 2025
 */

#include "drivers/ServoMotor.h"
#include <Arduino.h>
#include "SystemConfig.h"
#include "utils/Logger.h"

// =============================================================================
// Constructor
// =============================================================================

ServoMotor::ServoMotor(int pin, int closedAngle, int openAngle,
                       unsigned long moveDelay, unsigned long freeDelay)
    : pinServo(pin)
    , currentState(ServoControlState::IDLE)
    , stateStartTime(0)
    , isInitialized(false)
    , autoCycle(false)
    , cycleCount(0)
    , isEnergized(false)
    , angleClosed(closedAngle)
    , angleOpen(openAngle)
    , movementDelay(moveDelay)
    , freeDelay(freeDelay)
    , pwmChannel(0)
    , pwmFrequency(50)
    , pwmResolution(16)
    , minPulseWidth(500)
    , maxPulseWidth(2500)
{
}

// =============================================================================
// Métodos Privados
// =============================================================================

uint32_t ServoMotor::angleToDutyCycle(int angle) {
    // Limitar ángulo al rango válido
    angle = constrain(angle, 0, 180);
    
    // Convertir ángulo a microsegundos de pulso
    int pulseWidth = map(angle, 0, 180, minPulseWidth, maxPulseWidth);
    
    // Convertir microsegundos a duty cycle
    // Duty cycle = (pulseWidth * frequency * 2^resolution) / 1,000,000
    uint32_t dutyCycle = (pulseWidth * pwmFrequency * (1 << pwmResolution)) / 1000000;
    
    return dutyCycle;
}

void ServoMotor::energize() {
    if (!isEnergized) {
        ledcWrite(pwmChannel, angleToDutyCycle(getCurrentAngle()));
        isEnergized = true;
    }
}

void ServoMotor::deenergize() {
    if (isEnergized) {
        ledcWrite(pwmChannel, 0);
        isEnergized = false;
    }
}

int ServoMotor::getCurrentAngle() const {
    switch (currentState) {
        case ServoControlState::CLOSED:
        case ServoControlState::MOVING_TO_CLOSE:
            return angleClosed;
        case ServoControlState::OPEN:
        case ServoControlState::MOVING_TO_OPEN:
            return angleOpen;
        default:
            return angleClosed;
    }
}

// =============================================================================
// Implementación de ServoControllerInterface
// =============================================================================

bool ServoMotor::init() {
    LOG_INFO("[SERVO] Inicializando servomotor con PWM nativo ESP32...");
    
    try {
        // Configurar PWM para ESP32
        ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
        ledcAttachPin(pinServo, pwmChannel);
        
        // Mover a posición inicial (cerrada)
        energize();
        ledcWrite(pwmChannel, angleToDutyCycle(angleClosed));
        delay(movementDelay);
        
        // Desenergizar el servo
        deenergize();
        delay(freeDelay);
        
        currentState = ServoControlState::CLOSED;
        isInitialized = true;
        stateStartTime = millis();
        
        LOG_INFO("[SERVO] Servomotor inicializado correctamente");
        LOG_INFO("[SERVO] Pin: " + String(pinServo) + ", Frecuencia: " + String(pwmFrequency) + "Hz");
        LOG_INFO("[SERVO] Posición inicial: " + String(angleClosed) + "°");
        
        return true;
        
    } catch (...) {
        LOG_ERROR("[SERVO] Fallo en inicialización del servo");
        currentState = ServoControlState::ERROR;
        return false;
    }
}

void ServoMotor::startCycle(bool enableAutoCycle) {
    if (!isInitialized) {
        LOG_ERROR("[SERVO] Servo no inicializado. Llame a init() primero.");
        return;
    }
    
    autoCycle = enableAutoCycle;
    currentState = ServoControlState::MOVING_TO_OPEN;
    stateStartTime = millis();
    
    // Energizar y mover a posición abierta
    energize();
    ledcWrite(pwmChannel, angleToDutyCycle(angleOpen));
    
    LOG_INFO("[SERVO] Iniciando ciclo de movimiento");
    LOG_INFO("[SERVO] Auto-ciclo: " + String(autoCycle ? "Habilitado" : "Deshabilitado"));
}

void ServoMotor::stopCycle() {
    if (currentState != ServoControlState::IDLE && currentState != ServoControlState::CLOSED) {
        // Cerrar el servo antes de detener
        closeServo();
    }
    
    currentState = ServoControlState::IDLE;
    autoCycle = false;
    
    LOG_INFO("[SERVO] Ciclo detenido");
}

void ServoMotor::update() {
    if (!isInitialized || currentState == ServoControlState::IDLE) {
        return;
    }

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - stateStartTime;

    switch (currentState) {
        case ServoControlState::MOVING_TO_OPEN:
            if (elapsedTime >= movementDelay) {
                // Servo ya está en posición abierta, desenergizar
                deenergize();
                currentState = ServoControlState::OPEN;
                stateStartTime = currentTime;
                
                LOG_INFO("[SERVO] Movido a " + String(angleOpen) + "° - Desenergizado");
            }
            break;

        case ServoControlState::OPEN:
            if (elapsedTime >= freeDelay) {
                // Iniciar movimiento a posición cerrada
                energize();
                ledcWrite(pwmChannel, angleToDutyCycle(angleClosed));
                currentState = ServoControlState::MOVING_TO_CLOSE;
                stateStartTime = currentTime;
                
                LOG_INFO("[SERVO] Moviendo a " + String(angleClosed) + "°");
            }
            break;

        case ServoControlState::MOVING_TO_CLOSE:
            if (elapsedTime >= movementDelay) {
                // Servo ya está en posición cerrada, desenergizar
                deenergize();
                currentState = ServoControlState::CLOSED;
                stateStartTime = currentTime;
                cycleCount++;
                
                LOG_INFO("[SERVO] Movido a " + String(angleClosed) + "° - Desenergizado");
                LOG_INFO("[SERVO] Ciclo completado. Total: " + String(cycleCount));
                
                if (autoCycle) {
                    // Esperar tiempo libre antes del siguiente ciclo
                    delay(freeDelay);
                    // Reiniciar ciclo
                    startCycle(true);
                }
            }
            break;

        case ServoControlState::CLOSED:
            // En posición cerrada, esperar comandos
            break;

        case ServoControlState::ERROR:
            LOG_ERROR("[SERVO] Servo en estado de error");
            break;

        default:
            break;
    }
}

void ServoMotor::openServo() {
    if (!isInitialized) {
        LOG_ERROR("[SERVO] Servo no inicializado");
        return;
    }
    
    energize();
    ledcWrite(pwmChannel, angleToDutyCycle(angleOpen));
    currentState = ServoControlState::OPEN;
    stateStartTime = millis();
    
    LOG_INFO("[SERVO] Abierto manualmente a " + String(angleOpen) + "°");
}

void ServoMotor::closeServo() {
    if (!isInitialized) {
        LOG_ERROR("[SERVO] Servo no inicializado");
        return;
    }
    
    energize();
    ledcWrite(pwmChannel, angleToDutyCycle(angleClosed));
    currentState = ServoControlState::CLOSED;
    stateStartTime = millis();
    
    LOG_INFO("[SERVO] Cerrado manualmente a " + String(angleClosed) + "°");
}

ServoControlState ServoMotor::getState() const {
    return currentState;
}

unsigned long ServoMotor::getCycleCount() const {
    return cycleCount;
}

bool ServoMotor::isReady() const {
    return isInitialized;
}

unsigned long ServoMotor::getStateElapsedTime() const {
    return millis() - stateStartTime;
}

bool ServoMotor::isServoEnergized() const {
    return isEnergized;
}

void ServoMotor::printStatus() const {
    Serial.println("=== ESTADO DEL SERVOMOTOR ===");
    Serial.println("Estado: " + String(ServoControllerInterface::stateToString(currentState)));
    Serial.println("Pin: " + String(pinServo));
    Serial.println("Frecuencia PWM: " + String(pwmFrequency) + "Hz");
    Serial.println("Resolución PWM: " + String(pwmResolution) + " bits");
    Serial.println("Inicializado: " + String(isInitialized ? "Sí" : "No"));
    Serial.println("Energizado: " + String(isEnergized ? "Sí" : "No"));
    Serial.println("Auto-ciclo: " + String(autoCycle ? "Habilitado" : "Deshabilitado"));
    Serial.println("Ciclos completados: " + String(cycleCount));
    Serial.println("Tiempo en estado actual: " + String(getStateElapsedTime()) + "ms");
    Serial.println("Posición cerrada: " + String(angleClosed) + "°");
    Serial.println("Posición abierta: " + String(angleOpen) + "°");
    Serial.println("========================================");
}

// =============================================================================
// Métodos de Configuración Avanzada
// =============================================================================

void ServoMotor::setPulseLimits(int minPulse, int maxPulse) {
    minPulseWidth = minPulse;
    maxPulseWidth = maxPulse;
    
    LOG_INFO("[SERVO] Límites de pulso configurados:");
    LOG_INFO("  - Mínimo: " + String(minPulseWidth) + "μs");
    LOG_INFO("  - Máximo: " + String(maxPulseWidth) + "μs");
}

void ServoMotor::setPWMConfig(int channel, int frequency, int resolution) {
    pwmChannel = channel;
    pwmFrequency = frequency;
    pwmResolution = resolution;
    
    LOG_INFO("[SERVO] Configuración PWM actualizada:");
    LOG_INFO("  - Canal: " + String(pwmChannel));
    LOG_INFO("  - Frecuencia: " + String(pwmFrequency) + "Hz");
    LOG_INFO("  - Resolución: " + String(pwmResolution) + " bits");
}

bool ServoMotor::validateConfiguration() const {
    // Validar pin
    if (pinServo < 0 || pinServo > 39) {
        LOG_ERROR("[SERVO] Pin inválido: " + String(pinServo));
        return false;
    }
    
    // Validar frecuencia
    if (pwmFrequency < 1 || pwmFrequency > 40000) {
        LOG_ERROR("[SERVO] Frecuencia PWM inválida: " + String(pwmFrequency));
        return false;
    }
    
    // Validar resolución
    if (pwmResolution < 1 || pwmResolution > 16) {
        LOG_ERROR("[SERVO] Resolución PWM inválida: " + String(pwmResolution));
        return false;
    }
    
    // Validar ángulos
    if (angleClosed < 0 || angleClosed > 180 || angleOpen < 0 || angleOpen > 180) {
        LOG_ERROR("[SERVO] Ángulos inválidos");
        return false;
    }
    
    // Validar tiempos
    if (movementDelay == 0 || freeDelay == 0) {
        LOG_ERROR("[SERVO] Tiempos de movimiento inválidos");
        return false;
    }
    
    LOG_INFO("[SERVO] Configuración válida");
    return true;
}

// =============================================================================
// Instancia Global del ServoMotor
// =============================================================================

/**
 * @brief Instancia global del controlador de servo.
 * 
 * Esta instancia es utilizada por el sistema principal para controlar
 * el servomotor. La configuración por defecto usa el pin definido en
 * SystemConfig.h y los ángulos estándar.
 */
ServoMotor servoControlador(HardwareConfig::SERVO_PIN);
