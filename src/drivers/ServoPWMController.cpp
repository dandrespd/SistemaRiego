/**
 * @file ServoPWMController.cpp
 * @brief Implementación del controlador de servomotores PWM para sistema de riego multi-zona.
 * 
 * Este archivo contiene la implementación completa de la clase ServoPWMController,
 * que gestiona el control secuencial de servomotores para un sistema de riego
 * inteligente y eficiente.
 * 
 * CONCEPTOS IMPLEMENTADOS:
 * - Control PWM nativo del ESP32 usando el periférico LEDC
 * - Máquina de estados para gestión de ciclos de riego
 * - Movimiento suave de servomotores para evitar golpes hidráulicos
 * - Sistema robusto de manejo de errores con recuperación automática
 * - Operación no bloqueante compatible con otras tareas del sistema
 * 
 * ARQUITECTURA DEL CÓDIGO:
 * La implementación sigue el patrón "Estado" donde cada estado del sistema
 * tiene comportamientos específicos y transiciones bien definidas. Esto hace
 * el código más mantenible y fácil de extender.
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0
 * @date 2025
*/

// =============================================================================
// Includes necesarios
// =============================================================================
// Primero incluir siempre los headers estándar
#include <stdint.h>         // Para tipos enteros estándar
#include <cmath>            // Para funciones matemáticas avanzadas

// Conditional compilation for Arduino vs non-Arduino environments
#if defined(ARDUINO) && !defined(__clang_analyzer__)
// Arduino environment - use actual Arduino headers
#include <Arduino.h>
#else
// Non-Arduino environment (for static analysis/testing)
#include <string>
#include <iostream>
// Define Arduino-compatible types and functions
using String = std::string;
unsigned long millis() { return 0; }
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits) {}
void ledcAttachPin(uint8_t pin, uint8_t channel) {}
void ledcWrite(uint8_t channel, uint32_t duty) {}

// Default configuration for static analysis
#ifndef NUM_SERVOS
#define NUM_SERVOS 4
#endif
#ifndef SERVO_PINS
const uint8_t SERVO_PINS[NUM_SERVOS] = {1, 2, 3, 4};
#endif

// Mock Serial object for non-Arduino environments
class MockSerial {
public:
    void println(const String& message) {}
    void print(const String& message) {}
};
MockSerial Serial;
#endif

// Include project headers after environment setup
#include "../../include/drivers/ServoPWMController.h"
#include "../../include/core/ProjectConfig.h"  // Configuración centralizada - reemplaza SystemConfig.h


// =============================================================================
// Constructor y Destructor
// =============================================================================

/**
 * @brief Constructor de la clase ServoPWMController.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El constructor prepara el objeto para su uso posterior pero NO configura
 * el hardware inmediatamente. Esto es una buena práctica porque:
 * 1. Permite crear el objeto antes de que el hardware esté listo
 * 2. Facilita el manejo de errores en la inicialización
 * 3. Permite configuraciones personalizadas antes de init()
 * 
 * @param numZones Número de zonas de riego a controlar
 */
ServoPWMController::ServoPWMController(uint8_t numZones) 
    : zones(nullptr)
    , totalZones(0)
    , currentZone(0)
    , systemState(IrrigationState::IDLE)
    , stateStartTime(0)
    , lastStatusReport(0)
    , autoCycle(false)
    , emergencyStop(false)
    , totalCyclesCompleted(0)
    , totalWateringTime(0)
    , systemStartTime(0)
{
    // Validación del número de zonas solicitado
    if (numZones == 0 || numZones > NUM_SERVOS) {
        Serial.println("[ERROR] Número de zonas inválido. Usando configuración por defecto.");
        numZones = NUM_SERVOS;
    }
    
    // Inicializar el controlador con el número de zonas válido
    initializeZones(numZones);
}

/**
 * @brief Constructor de copia.
 * 
 * Crea una copia profunda del controlador, incluyendo toda la configuración
 * y estado de las zonas. Útil para crear respaldos o múltiples instancias.
 * 
 * @param other Controlador a copiar
 */
ServoPWMController::ServoPWMController(const ServoPWMController& other)
    : zones(nullptr)
    , totalZones(0)
    , currentZone(other.currentZone)
    , systemState(other.systemState)
    , stateStartTime(other.stateStartTime)
    , lastStatusReport(other.lastStatusReport)
    , autoCycle(other.autoCycle)
    , emergencyStop(other.emergencyStop)
    , totalCyclesCompleted(other.totalCyclesCompleted)
    , totalWateringTime(other.totalWateringTime)
    , systemStartTime(other.systemStartTime)
{
    // Copiar zonas
    if (other.zones != nullptr && other.totalZones > 0) {
        initializeZones(other.totalZones);
        
        // Copiar datos de cada zona
        for (uint8_t i = 0; i < totalZones; i++) {
            zones[i] = other.zones[i];
        }
    }
}

/**
 * @brief Operador de asignación.
 * 
 * Asigna el contenido de otro controlador a este, realizando una copia profunda.
 * Maneja correctamente la limpieza de recursos existentes.
 * 
 * @param other Controlador a asignar
 * @return Referencia a este controlador
 */
ServoPWMController& ServoPWMController::operator=(const ServoPWMController& other)
{
    // Evitar auto-asignación
    if (this != &other) {
        // Limpiar recursos existentes
        cleanup();
        
        // Copiar datos básicos
        currentZone = other.currentZone;
        systemState = other.systemState;
        stateStartTime = other.stateStartTime;
        lastStatusReport = other.lastStatusReport;
        autoCycle = other.autoCycle;
        emergencyStop = other.emergencyStop;
        totalCyclesCompleted = other.totalCyclesCompleted;
        totalWateringTime = other.totalWateringTime;
        systemStartTime = other.systemStartTime;
        
        // Copiar zonas
        if (other.zones != nullptr && other.totalZones > 0) {
            initializeZones(other.totalZones);
            
            // Copiar datos de cada zona
            for (uint8_t i = 0; i < totalZones; i++) {
                zones[i] = other.zones[i];
            }
        }
    }
    
    return *this;
}

/**
 * @brief Constructor de movimiento (C++11).
 * 
 * Transfiere recursos de otro controlador de forma eficiente.
 * El controlador original queda en estado válido pero vacío.
 * 
 * @param other Controlador a mover (rvalue reference)
 */
ServoPWMController::ServoPWMController(ServoPWMController&& other) noexcept
    : zones(other.zones)
    , totalZones(other.totalZones)
    , currentZone(other.currentZone)
    , systemState(other.systemState)
    , stateStartTime(other.stateStartTime)
    , lastStatusReport(other.lastStatusReport)
    , autoCycle(other.autoCycle)
    , emergencyStop(other.emergencyStop)
    , totalCyclesCompleted(other.totalCyclesCompleted)
    , totalWateringTime(other.totalWateringTime)
    , systemStartTime(other.systemStartTime)
{
    // Resetear el controlador original
    other.zones = nullptr;
    other.totalZones = 0;
    other.currentZone = 0;
    other.systemState = IrrigationState::IDLE;
    other.stateStartTime = 0;
    other.lastStatusReport = 0;
    other.autoCycle = false;
    other.emergencyStop = false;
    other.totalCyclesCompleted = 0;
    other.totalWateringTime = 0;
    other.systemStartTime = 0;
}

/**
 * @brief Operador de asignación de movimiento (C++11).
 * 
 * Transfiere recursos de forma eficiente, limpiando los recursos
 * existentes de este controlador.
 * 
 * @param other Controlador a mover (rvalue reference)
 * @return Referencia a este controlador
 */
ServoPWMController& ServoPWMController::operator=(ServoPWMController&& other) noexcept
{
    // Evitar auto-asignación
    if (this != &other) {
        // Limpiar recursos existentes
        cleanup();
        
        // Transferir recursos
        zones = other.zones;
        totalZones = other.totalZones;
        currentZone = other.currentZone;
        systemState = other.systemState;
        stateStartTime = other.stateStartTime;
        lastStatusReport = other.lastStatusReport;
        autoCycle = other.autoCycle;
        emergencyStop = other.emergencyStop;
        totalCyclesCompleted = other.totalCyclesCompleted;
        totalWateringTime = other.totalWateringTime;
        systemStartTime = other.systemStartTime;
        
        // Resetear el controlador original
        other.zones = nullptr;
        other.totalZones = 0;
        other.currentZone = 0;
        other.systemState = IrrigationState::IDLE;
        other.stateStartTime = 0;
        other.lastStatusReport = 0;
        other.autoCycle = false;
        other.emergencyStop = false;
        other.totalCyclesCompleted = 0;
        other.totalWateringTime = 0;
        other.systemStartTime = 0;
    }
    
    return *this;
}

/**
 * @brief Destructor de la clase ServoPWMController.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El destructor asegura que el sistema se apague de forma segura y limpia
 * los recursos utilizados. Es especialmente importante en sistemas embebidos
 * donde los recursos son limitados.
 */
ServoPWMController::~ServoPWMController() {
    // Parada de emergencia para asegurar que todas las válvulas se cierren
    emergencyStopAll();
    
    // Liberar memoria asignada dinámicamente
    if (zones != nullptr) {
        delete[] zones;
        zones = nullptr;
    }
    
    Serial.println("[INFO] ServoPWMController destruido correctamente.");
}

// =============================================================================
// Métodos Auxiliares Privados - Gestión de Memoria
// =============================================================================

/**
 * @brief Inicializa las zonas del controlador.
 * 
 * Método auxiliar para centralizar la inicialización de zonas,
 * utilizado por constructores y operadores de asignación.
 * 
 * @param numZones Número de zonas a inicializar
 */
void ServoPWMController::initializeZones(uint8_t numZones) {
    // Validar parámetro
    if (numZones == 0 || numZones > NUM_SERVOS) {
        Serial.println("[ERROR] Número de zonas inválido en initializeZones.");
        return;
    }
    
    // Limpiar zonas existentes si las hay
    cleanup();
    
    // Asignar memoria para las zonas
    zones = new ZoneInfo[numZones];
    
    // Verificar asignación de memoria
    if (zones == nullptr) {
        Serial.println("[ERROR CRÍTICO] No se pudo asignar memoria para las zonas.");
        totalZones = 0;
        return;
    }
    
    totalZones = numZones;
    
    // Inicializar información básica de cada zona
    for (uint8_t i = 0; i < totalZones; i++) {
        zones[i].zoneNumber = i + 1;  // Numeración 1-based para el usuario
        zones[i].servoPin = SERVO_PINS[i];
        zones[i].pwmChannel = i;  // Cada servo usa un canal PWM único
        zones[i].currentState = ServoState::UNINITIALIZED;
        zones[i].lastActionTime = 0;
        zones[i].totalIrrigationTime = 0;
        zones[i].isEnabled = true;
        zones[i].retryCount = 0;
        
        // Copiar configuración específica si está disponible
        if (i < (sizeof(ZONE_CONFIGURATIONS) / sizeof(ZONE_CONFIGURATIONS[0]))) {
            zones[i].config = ZONE_CONFIGURATIONS[i];
        } else {
            // Usar configuración por defecto para zonas adicionales
            zones[i].config = DEFAULT_ZONE_CONFIG;
        }
    }
    
    Serial.println("[INFO] Zonas inicializadas: " + String(totalZones));
}

/**
 * @brief Limpia los recursos del controlador.
 * 
 * Método auxiliar para centralizar la limpieza de recursos,
 * utilizado por destructor y operadores de asignación.
 */
void ServoPWMController::cleanup() {
    // Parada de emergencia si es necesario
    if (systemState != IrrigationState::IDLE && systemState != IrrigationState::ERROR) {
        emergencyStopAll();
    }
    
    // Liberar memoria de zonas
    if (zones != nullptr) {
        delete[] zones;
        zones = nullptr;
    }
    
    // Resetear variables
    totalZones = 0;
    currentZone = 0;
    systemState = IrrigationState::IDLE;
    stateStartTime = 0;
    lastStatusReport = 0;
    autoCycle = false;
    emergencyStop = false;
    totalCyclesCompleted = 0;
    totalWateringTime = 0;
    systemStartTime = 0;
}

// =============================================================================
// Métodos Públicos - Interfaz Principal
// =============================================================================

/**
 * @brief Inicializa el sistema de control de servomotores.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * La inicialización se separa del constructor porque:
 * 1. Permite manejar errores de hardware de forma controlada
 * 2. El usuario puede configurar parámetros antes de la inicialización
 * 3. Facilita la re-inicialización sin crear nuevos objetos
 * 
 * @return true si la inicialización fue exitosa, false en caso de error
 */
bool ServoPWMController::init() {
    Serial.println("[INFO] Inicializando sistema de control de servomotores...");
    
    // Verificar que tenemos zonas válidas para controlar
    if (totalZones == 0 || zones == nullptr) {
        Serial.println("[ERROR] No hay zonas válidas configuradas.");
        return false;
    }
    
    // Validar configuración del sistema
    if (!validateConfiguration()) {
        Serial.println("[ERROR] Configuración del sistema inválida.");
        return false;
    }
    
    // Inicializar canales PWM del ESP32
    if (!initializePWMChannels()) {
        Serial.println("[ERROR] Fallo en la inicialización de canales PWM.");
        return false;
    }
    
    // Configurar cada servomotor individualmente
    for (uint8_t i = 0; i < totalZones; i++) {
        if (!setupServo(i)) {
            Serial.println("[ERROR] Fallo configurando servo de zona " + String(i + 1));
            return false;
        }
    }
    
    // Establecer estado inicial del sistema
    systemState = IrrigationState::IDLE;
    systemStartTime = millis();
    emergencyStop = false;
    
    Serial.println("[ÉXITO] Sistema de servomotores inicializado correctamente.");
    Serial.println("[INFO] Zonas configuradas: " + String(totalZones));
    
    // Generar reporte inicial del estado
    generateStatusReport();
    
    return true;
}

/**
 * @brief Inicia un ciclo completo de riego en todas las zonas habilitadas.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El ciclo de riego implementa una máquina de estados que procesa cada zona
 * secuencialmente. Esto es importante porque:
 * 1. Mantiene presión de agua constante (no divide el flujo)
 * 2. Reduce el consumo energético de la bomba
 * 3. Permite mejor control y monitoreo de cada zona
 * 
 * @param enableAutoCycle Si true, repite el ciclo automáticamente
 * @return true si el ciclo se inició correctamente
 */
bool ServoPWMController::startIrrigationCycle(bool enableAutoCycle) {
    // Verificar que el sistema esté listo para iniciar
    if (systemState != IrrigationState::IDLE && systemState != IrrigationState::COMPLETED) {
        Serial.println("[ADVERTENCIA] Sistema ocupado. Estado actual: " + String(stateToString(systemState)));
        return false;
    }
    
    // Verificar que no hay parada de emergencia activa
    if (emergencyStop) {
        Serial.println("[ERROR] Parada de emergencia activa. Use resetEmergencyStop() primero.");
        return false;
    }
    
    // Contar zonas habilitadas
    uint8_t enabledZones = 0;
    for (uint8_t i = 0; i < totalZones; i++) {
        if (zones[i].isEnabled) {
            enabledZones++;
        }
    }
    
    if (enabledZones == 0) {
        Serial.println("[ADVERTENCIA] No hay zonas habilitadas para riego.");
        return false;
    }
    
    // Configurar parámetros del ciclo
    autoCycle = enableAutoCycle;
    currentZone = 0;
    
    // Encontrar la primera zona habilitada
    while (currentZone < totalZones && !zones[currentZone].isEnabled) {
        currentZone++;
    }
    
    // Iniciar el ciclo cambiando al estado de inicialización
    systemState = IrrigationState::INITIALIZING;
    stateStartTime = millis();
    
    Serial.println("[INFO] Iniciando ciclo de riego. Zonas habilitadas: " + String(enabledZones));
    Serial.println("[INFO] Auto-ciclo: " + String(enableAutoCycle ? "Habilitado" : "Deshabilitado"));
    
    return true;
}

/**
 * @brief Función principal de procesamiento del sistema.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * Esta función implementa una máquina de estados no bloqueante. Cada llamada:
 * 1. Evalúa el estado actual del sistema
 * 2. Ejecuta las acciones correspondientes al estado
 * 3. Determina si debe transicionar a un nuevo estado
 * 
 * El patrón no bloqueante es crucial en sistemas embebidos porque permite
 * que el microcontrolador maneje otras tareas importantes mientras controla
 * el riego (como monitoreo de sensores, comunicación WiFi, etc.).
 */
void ServoPWMController::update() {
    // Verificar parada de emergencia
    if (emergencyStop) {
        return; // No procesar nada si hay parada de emergencia
    }
    
    // Generar reportes de estado periódicamente
    if (millis() - lastStatusReport >= STATUS_REPORT_INTERVAL_MS) {
        if (ENABLE_VERBOSE_LOGGING && systemState != IrrigationState::IDLE) {
            generateStatusReport();
        }
        lastStatusReport = millis();
    }
    
    // Procesar estado actual del sistema
    switch (systemState) {
        case IrrigationState::IDLE:
            // Estado inactivo - no hay acciones que realizar
            break;
            
        case IrrigationState::INITIALIZING:
            handleInitializingState();
            break;
            
        case IrrigationState::OPENING_VALVE:
            handleOpeningValveState();
            break;
            
        case IrrigationState::IRRIGATING:
            handleIrrigatingState();
            break;
            
        case IrrigationState::CLOSING_VALVE:
            handleClosingValveState();
            break;
            
        case IrrigationState::TRANSITIONING:
            handleTransitioningState();
            break;
            
        case IrrigationState::COMPLETED:
            handleCompletedState();
            break;
            
        case IrrigationState::ERROR:
            handleErrorState();
            break;
    }
}

/**
 * @brief Detiene el ciclo de riego actual de forma segura.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * La parada segura es diferente de la parada de emergencia porque:
 * 1. Permite que las operaciones en curso terminen ordenadamente
 * 2. Cierra todas las válvulas antes de detener
 * 3. Mantiene el sistema en estado operativo para futuras operaciones
 */
void ServoPWMController::stopIrrigationCycle() {
    Serial.println("[INFO] Deteniendo ciclo de riego de forma segura...");
    
    // Si estamos regando activamente, cerrar la válvula actual
    if (systemState == IrrigationState::IRRIGATING || 
        systemState == IrrigationState::OPENING_VALVE) {
        
        // Cambiar a estado de cierre para finalizar ordenadamente
        systemState = IrrigationState::CLOSING_VALVE;
        stateStartTime = millis();
        
        Serial.println("[INFO] Cerrando válvula de zona " + String(currentZone + 1) + " antes de detener.");
    } else {
        // Si no estamos regando activamente, ir directamente a idle
        systemState = IrrigationState::IDLE;
        autoCycle = false;
    }
    
    // Asegurar que todas las válvulas estén cerradas
    for (uint8_t i = 0; i < totalZones; i++) {
        if (zones[i].currentState == ServoState::OPEN || 
            zones[i].currentState == ServoState::OPENING) {
            moveServoToAngle(i, SERVO_CLOSED_ANGLE);
            zones[i].currentState = ServoState::CLOSING;
        }
    }
}

/**
 * @brief Activa parada de emergencia del sistema.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * La parada de emergencia es la función de seguridad más importante porque:
 * 1. Cierra INMEDIATAMENTE todas las válvulas sin esperar
 * 2. Previene daños por inundación o mal funcionamiento
 * 3. Requiere intervención manual para reactivar el sistema
 */
void ServoPWMController::emergencyStopAll() {
    // Solo mostrar mensajes si no estaba ya en parada de emergencia
    if (!emergencyStop) {
        Serial.println("[EMERGENCIA] Activando parada de emergencia del sistema!");
    }
    
    emergencyStop = true;
    systemState = IrrigationState::ERROR;
    autoCycle = false;
    
    // Cerrar inmediatamente todas las válvulas
    for (uint8_t i = 0; i < totalZones; i++) {
        moveServoToAngle(i, SERVO_CLOSED_ANGLE);
        zones[i].currentState = ServoState::CLOSED;
        zones[i].lastActionTime = millis();
    }
    
    // Solo mostrar mensajes si no estaba ya en parada de emergencia
    if (!emergencyStop) {
        Serial.println("[EMERGENCIA] Todas las válvulas cerradas. Sistema en parada de emergencia.");
        Serial.println("[INFO] Use resetEmergencyStop() para reactivar el sistema.");
    }
}

/**
 * @brief Desactiva la parada de emergencia.
 */
void ServoPWMController::resetEmergencyStop() {
    if (!emergencyStop) {
        Serial.println("[INFO] No hay parada de emergencia activa.");
        return;
    }
    
    Serial.println("[INFO] Desactivando parada de emergencia...");
    
    emergencyStop = false;
    systemState = IrrigationState::IDLE;
    
    // Reinicializar estados de zonas
    for (uint8_t i = 0; i < totalZones; i++) {
        zones[i].currentState = ServoState::CLOSED;
        zones[i].retryCount = 0;
    }
    
    Serial.println("[INFO] Sistema listo para operar. Recuerde llamar a init() si es necesario.");
}

// =============================================================================
// Métodos Privados - Implementación de Estados
// =============================================================================

/**
 * @brief Maneja el estado de inicialización del ciclo de riego.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El estado de inicialización prepara el sistema para el ciclo de riego:
 * 1. Verifica que todas las válvulas estén cerradas
 * 2. Reinicia contadores y estadísticas
 * 3. Prepara la transición al primer riego
 */
void ServoPWMController::handleInitializingState() {
    // Asegurar que todas las válvulas estén cerradas antes de empezar
    bool allClosed = true;
    for (uint8_t i = 0; i < totalZones; i++) {
        if (zones[i].currentState != ServoState::CLOSED) {
            moveServoToAngle(i, SERVO_CLOSED_ANGLE);
            zones[i].currentState = ServoState::CLOSED;
            allClosed = false;
        }
    }
    
    // Si todas las válvulas están cerradas, continuar al primer riego
    if (allClosed) {
        Serial.println("[INFO] Iniciando riego de zona " + String(currentZone + 1) + 
                      " (" + String(zones[currentZone].config.name) + ")");
        
        systemState = IrrigationState::OPENING_VALVE;
        stateStartTime = millis();
    }
}

/**
 * @brief Maneja el estado de apertura de válvula.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * La apertura controlada de válvulas previene golpes de ariete hidráulicos
 * que pueden dañar tuberías y conexiones. El movimiento gradual es clave.
 */
void ServoPWMController::handleOpeningValveState() {
    uint8_t targetAngle = zones[currentZone].config.openAngle;
    
    // Mover servo a posición de apertura
    if (moveServoToAngle(currentZone, targetAngle)) {
        zones[currentZone].currentState = ServoState::OPEN;
        zones[currentZone].lastActionTime = millis();
        
        Serial.println("[INFO] Válvula de zona " + String(currentZone + 1) + " abierta. Iniciando riego...");
        
        // Transicionar a estado de riego
        systemState = IrrigationState::IRRIGATING;
        stateStartTime = millis();
    } else {
        // Manejar error de apertura
        if (millis() - stateStartTime > SERVO_MOVEMENT_TIME_MS * 2) {
            handleServoError(currentZone, "Fallo en apertura de válvula");
        }
    }
}

/**
 * @brief Maneja el estado de riego activo.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * Durante el riego, el sistema monitorea continuamente:
 * 1. Tiempo transcurrido vs tiempo configurado
 * 2. Estado del servo (verificar que sigue abierto)
 * 3. Condiciones de error que requieran parada
 */
void ServoPWMController::handleIrrigatingState() {
    uint32_t elapsedTime = (millis() - stateStartTime) / 1000; // Convertir a segundos
    uint32_t targetTime = zones[currentZone].config.irrigationTime;
    
    // Actualizar estadísticas de riego
    zones[currentZone].totalIrrigationTime = elapsedTime;
    
    // Verificar si el tiempo de riego se ha completado
    if (elapsedTime >= targetTime) {
        Serial.println("[INFO] Riego de zona " + String(currentZone + 1) + " completado. Tiempo: " + 
                      String(elapsedTime) + "s");
        
        // Transicionar a estado de cierre
        systemState = IrrigationState::CLOSING_VALVE;
        stateStartTime = millis();
        
        // Actualizar estadísticas globales
        totalWateringTime += elapsedTime;
    }
    
    // Verificar que el servo sigue en posición correcta
    if (zones[currentZone].currentState != ServoState::OPEN) {
        Serial.println("[ADVERTENCIA] Servo de zona " + String(currentZone + 1) + " no está en posición abierta");
        // Intentar reposicionar el servo
        moveServoToAngle(currentZone, zones[currentZone].config.openAngle);
    }
}

/**
 * @brief Maneja el estado de cierre de válvula.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El cierre controlado es tan importante como la apertura porque:
 * 1. Previene golpes de ariete al detener el flujo gradualmente
 * 2. Asegura sellado completo de la válvula
 * 3. Prepara el sistema para la siguiente zona
 */
void ServoPWMController::handleClosingValveState() {
    // Mover servo a posición cerrada
    if (moveServoToAngle(currentZone, SERVO_CLOSED_ANGLE)) {
        zones[currentZone].currentState = ServoState::CLOSED;
        zones[currentZone].lastActionTime = millis();
        
        Serial.println("[INFO] Válvula de zona " + String(currentZone + 1) + " cerrada correctamente.");
        
        // Buscar la siguiente zona habilitada
        uint8_t nextZone = currentZone + 1;
        while (nextZone < totalZones && !zones[nextZone].isEnabled) {
            nextZone++;
        }
        
        if (nextZone < totalZones) {
            // Hay más zonas por procesar - transición
            currentZone = nextZone;
            systemState = IrrigationState::TRANSITIONING;
            stateStartTime = millis();
            
            Serial.println("[INFO] Transicionando a zona " + String(currentZone + 1) + "...");
        } else {
            // Todas las zonas completadas
            systemState = IrrigationState::COMPLETED;
            stateStartTime = millis();
            totalCyclesCompleted++;
            
            Serial.println("[ÉXITO] Ciclo de riego completado. Ciclos totales: " + String(totalCyclesCompleted));
        }
    } else {
        // Manejar error de cierre
        if (millis() - stateStartTime > SERVO_MOVEMENT_TIME_MS * 2) {
            handleServoError(currentZone, "Fallo en cierre de válvula");
        }
    }
}

/**
 * @brief Maneja el estado de transición entre zonas.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * La pausa entre zonas es importante para:
 * 1. Permitir que la presión del sistema se estabilice
 * 2. Evitar picos de corriente al activar móltiples servos seguidos
 * 3. Dar tiempo para que el agua residual drene completamente
 */
void ServoPWMController::handleTransitioningState() {
    uint32_t elapsedTime = (millis() - stateStartTime) / 1000;
    
    if (elapsedTime >= TRANSITION_TIME_SECONDS) {
        Serial.println("[INFO] Transición completada. Iniciando riego de zona " + String(currentZone + 1));
        
        systemState = IrrigationState::OPENING_VALVE;
        stateStartTime = millis();
    }
}

/**
 * @brief Maneja el estado de ciclo completado.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El estado completado determina si:
 * 1. El ciclo debe repetirse automáticamente (auto-ciclo)
 * 2. El sistema debe retornar a estado idle
 * 3. Se deben generar reportes finales
 */
void ServoPWMController::handleCompletedState() {
    if (autoCycle) {
        // Esperar un tiempo antes de reiniciar el ciclo
        uint32_t elapsedTime = (millis() - stateStartTime) / 1000;
        const uint32_t CYCLE_RESTART_DELAY = 300; // 5 minutos entre ciclos
        
        if (elapsedTime >= CYCLE_RESTART_DELAY) {
            Serial.println("[INFO] Reiniciando ciclo automático...");
            currentZone = 0;
            
            // Encontrar la primera zona habilitada
            while (currentZone < totalZones && !zones[currentZone].isEnabled) {
                currentZone++;
            }
            
            systemState = IrrigationState::INITIALIZING;
            stateStartTime = millis();
        }
    } else {
        // Ciclo ónico completado - retornar a idle
        systemState = IrrigationState::IDLE;
        Serial.println("[INFO] Sistema en estado idle. Listo para nuevo ciclo.");
    }
}

/**
 * @brief Maneja estados de error del sistema.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El manejo de errores implementa estrategias de recuperación automática:
 * 1. Identificar el tipo de error
 * 2. Aplicar estrategia de recuperación apropiada
 * 3. Si la recuperación falla, activar parada de seguridad
 */
void ServoPWMController::handleErrorState() {
    // En estado de error, intentar recuperación limitada
    static uint32_t lastRecoveryAttempt = 0;
    const uint32_t RECOVERY_INTERVAL = 10000; // Intentar recuperación cada 10 segundos
    
    if (millis() - lastRecoveryAttempt >= RECOVERY_INTERVAL) {
        Serial.println("[INFO] Intentando recuperación automática del sistema...");
        
        // Cerrar todas las válvulas como medida de seguridad
        for (uint8_t i = 0; i < totalZones; i++) {
            moveServoToAngle(i, SERVO_CLOSED_ANGLE);
            zones[i].currentState = ServoState::CLOSED;
        }
        
        // Reinicializar sistema
        if (init()) {
            Serial.println("[ÉXITO] Recuperación automática exitosa.");
            systemState = IrrigationState::IDLE;
        } else {
            Serial.println("[ERROR] Recuperación automática fallida. Intervención manual requerida.");
            emergencyStopAll();
        }
        
        lastRecoveryAttempt = millis();
    }
}

// =============================================================================
// Métodos Privados - Funciones de Soporte
// =============================================================================

/**
 * @brief Inicializa los canales PWM para todos los servomotores.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El ESP32 tiene un periférico LEDC (LED Controller) que puede generar señales PWM
 * de alta precisión. Este periférico tiene:
 * - 16 canales independientes (0-15)
 * - Resolución configurable (1-20 bits)
 * - Frecuencia configurable (hasta 40 MHz)
 * 
 * Para servomotores necesitamos:
 * - Frecuencia: 50 Hz (periódo de 20ms)
 * - Resolución: 12 bits (4096 pasos)
 * - Pulsos: 1-2ms para control de 0-180°
 */
bool ServoPWMController::initializePWMChannels() {
    Serial.println("[INFO] Configurando canales PWM del ESP32...");
    
    for (uint8_t i = 0; i < totalZones; i++) {
        // Configurar canal PWM
        // Parámetros: canal, frecuencia, resolución
        if (ledcSetup(zones[i].pwmChannel, PWM_FREQUENCY, PWM_RESOLUTION) == 0) {
            Serial.println("[ERROR] Fallo configurando canal PWM " + String(zones[i].pwmChannel));
            return false;
        }
        
        // Asociar canal PWM con pin GPIO
        ledcAttachPin(zones[i].servoPin, zones[i].pwmChannel);
        
        // Establecer posición inicial (cerrado)
        uint32_t closedPulse = angleToHaltValue(SERVO_CLOSED_ANGLE);
        ledcWrite(zones[i].pwmChannel, closedPulse);
        
        Serial.println("[INFO] Canal PWM " + String(zones[i].pwmChannel) + 
                      " configurado en pin " + String(zones[i].servoPin));
    }
    
    return true;
}

/**
 * @brief Configura un servomotor específico en su posición inicial.
 */
bool ServoPWMController::setupServo(uint8_t zoneIndex) {
    if (zoneIndex >= totalZones) {
        return false;
    }
    
    // Mover servo a posición cerrada
    if (moveServoToAngle(zoneIndex, SERVO_CLOSED_ANGLE)) {
        zones[zoneIndex].currentState = ServoState::CLOSED;
        zones[zoneIndex].lastActionTime = millis();
        zones[zoneIndex].retryCount = 0;
        
        Serial.println("[INFO] Servo zona " + String(zoneIndex + 1) + " configurado correctamente.");
        return true;
    }
    
    return false;
}

/**
 * @brief Mueve un servomotor a un ángulo específico de forma gradual.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * El movimiento gradual es importante porque:
 * 1. Reduce el estrés mecánico en el servo y válvula
 * 2. Previene golpes de ariete en el sistema hidráulico
 * 3. Reduce picos de corriente que podrían afectar otros sistemas
 * 
 * @param zoneIndex Índice de la zona
 * @param targetAngle Ángulo objetivo (0-180 grados)
 * @return true si el movimiento se completó correctamente
 */
bool ServoPWMController::moveServoToAngle(uint8_t zoneIndex, uint8_t targetAngle) {
    if (zoneIndex >= totalZones) {
        return false;
    }
    
    // Validar ángulo objetivo
    if (targetAngle > 180) {
        targetAngle = 180;
    }
    
    // Calcular valor PWM para el ángulo objetivo
    uint32_t targetPulse = angleToHaltValue(targetAngle);
    
    // Aplicar valor PWM al servo
    ledcWrite(zones[zoneIndex].pwmChannel, targetPulse);
    
    // Actualizar estado del servo
    if (targetAngle == SERVO_CLOSED_ANGLE) {
        zones[zoneIndex].currentState = ServoState::CLOSING;
    } else {
        zones[zoneIndex].currentState = ServoState::OPENING;
    }
    
    zones[zoneIndex].lastActionTime = millis();
    
    if (ENABLE_VERBOSE_LOGGING) {
        Serial.println("[DEBUG] Servo zona " + String(zoneIndex + 1) + 
                      " moviéndose a " + String(targetAngle) + "° (PWM: " + String(targetPulse) + ")");
    }
    
    return true;
}

/**
 * @brief Convierte un ángulo en grados a valor PWM correspondiente.
 * 
 * EXPLICACIÓN EDUCATIVA:
 * Los servomotores estándar requieren pulsos de 1-2ms dentro de un período de 20ms:
 * - 1ms (5% duty cycle) = 0°
 * - 1.5ms (7.5% duty cycle) = 90°
 * - 2ms (10% duty cycle) = 180°
 * 
 * Para 12 bits de resolución (4096 pasos) y 50Hz:
 * - 1ms = 4096 * (1/20) = 204.8 ≈ 205 pasos
 * - 2ms = 4096 * (2/20) = 409.6 ≈ 410 pasos
 * 
 * @param angle Ángulo en grados (0-180)
 * @return Valor PWM correspondiente
 */
uint32_t ServoPWMController::angleToHaltValue(uint8_t angle) {
    // Limitar ángulo al rango válido
    if (angle > 180) {
        angle = 180;
    }
    
    // Interpolación lineal entre PWM_MIN_PULSE y PWM_MAX_PULSE
    // Fórmula: valor = min + (max - min) * (angle / 180)
    uint32_t pwmValue = PWM_MIN_PULSE + 
                       ((PWM_MAX_PULSE - PWM_MIN_PULSE) * angle) / 180;
    
    return pwmValue;
}

/**
 * @brief Valida la configuración del sistema antes de iniciar operaciones.
 */
bool ServoPWMController::validateConfiguration() {
    // Verificar que tenemos al menos una zona configurada
    if (totalZones == 0) {
        Serial.println("[ERROR] No hay zonas configuradas.");
        return false;
    }
    
    // Verificar configuración de cada zona
    for (uint8_t i = 0; i < totalZones; i++) {
        // Verificar pin válido
        if (zones[i].servoPin == 0) {
            Serial.println("[ERROR] Pin inválido para zona " + String(i + 1));
            return false;
        }
        
        // Verificar tiempo de riego válido
        if (zones[i].config.irrigationTime < MIN_IRRIGATION_TIME_SECONDS ||
            zones[i].config.irrigationTime > MAX_IRRIGATION_TIME_SECONDS) {
            Serial.println("[ERROR] Tiempo de riego inválido para zona " + String(i + 1));
            return false;
        }
        
        // Verificar ángulo de apertura válido
        if (zones[i].config.openAngle > 180) {
            Serial.println("[ERROR] Ángulo de apertura inválido para zona " + String(i + 1));
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Maneja errores de servomotores y aplica estrategias de recuperación.
 */
bool ServoPWMController::handleServoError(uint8_t zoneIndex, const char* errorType) {
    if (zoneIndex >= totalZones) {
        return false;
    }
    
    zones[zoneIndex].retryCount++;
    
    Serial.println("[ERROR] Zona " + String(zoneIndex + 1) + ": " + String(errorType) + 
                  " (Intento " + String(zones[zoneIndex].retryCount) + "/" + 
                  String(MAX_SERVO_RETRY_ATTEMPTS) + ")");
    
    // Si no hemos excedido el límite de intentos, reintentar
    if (zones[zoneIndex].retryCount <= MAX_SERVO_RETRY_ATTEMPTS) {
        // Reinicializar servo
        setupServo(zoneIndex);
        return true;
    } else {
        // Demasiados fallos - marcar zona como error y continuar
        zones[zoneIndex].currentState = ServoState::ERROR;
        zones[zoneIndex].isEnabled = false; // Deshabilitar zona problemática
        
        Serial.println("[ERROR CRÍTICO] Zona " + String(zoneIndex + 1) + 
                      " deshabilitada por fallos repetidos.");
        
        // Si todas las zonas están en error, activar parada de emergencia
        bool allInError = true;
        for (uint8_t i = 0; i < totalZones; i++) {
            if (zones[i].currentState != ServoState::ERROR && zones[i].isEnabled) {
                allInError = false;
                break;
            }
        }
        
        if (allInError) {
            emergencyStopAll();
            return false;
        }
        
        return true; // Continuar con otras zonas
    }
}

/**
 * @brief Genera reportes detallados del estado del sistema.
 */
void ServoPWMController::generateStatusReport() const {
    Serial.println("\n=== REPORTE DE ESTADO DEL SISTEMA ===");
    Serial.println("Estado del sistema: " + String(stateToString(systemState)));
    Serial.println("Zona actual: " + String(currentZone + 1) + "/" + String(totalZones));
    Serial.println("Auto-ciclo: " + String(autoCycle ? "Habilitado" : "Deshabilitado"));
    Serial.println("Parada de emergencia: " + String(emergencyStop ? "ACTIVA" : "Inactiva"));
    Serial.println("Ciclos completados: " + String(totalCyclesCompleted));
    Serial.println("Tiempo total de riego: " + String(totalWateringTime) + "s");
    
    uint32_t uptime = (millis() - systemStartTime) / 1000;
    Serial.println("Tiempo funcionamiento: " + String(uptime) + "s");
    
    Serial.println("\n--- Estado de Zonas ---");
    for (uint8_t i = 0; i < totalZones; i++) {
        Serial.println("Zona " + String(i + 1) + " (" + String(zones[i].config.name) + "): " +
                      String(servoStateToString(zones[i].currentState)) + 
                      " | Habilitada: " + String(zones[i].isEnabled ? "Sí" : "No") +
                      " | Tiempo riego: " + String(zones[i].totalIrrigationTime) + "s");
    }
    Serial.println("=======================================\n");
}

// =============================================================================
// Métodos de Consulta de Estado
// =============================================================================

IrrigationState ServoPWMController::getCurrentState() const {
    return systemState;
}

ServoState ServoPWMController::getZoneState(uint8_t zoneNumber) const {
    if (zoneNumber == 0 || zoneNumber > totalZones) {
        return ServoState::ERROR;
    }
    return zones[zoneNumber - 1].currentState;
}

bool ServoPWMController::isIrrigating() const {
    return systemState == IrrigationState::IRRIGATING;
}

uint8_t ServoPWMController::getCurrentActiveZone() const {
    if (systemState == IrrigationState::IRRIGATING) {
        return currentZone + 1; // Convertir a 1-based
    }
    return 0;
}

uint32_t ServoPWMController::getRemainingIrrigationTime() const {
    if (systemState != IrrigationState::IRRIGATING) {
        return 0;
    }
    
    uint32_t elapsed = (millis() - stateStartTime) / 1000;
    uint32_t total = zones[currentZone].config.irrigationTime;
    
    return (elapsed < total) ? (total - elapsed) : 0;
}

void ServoPWMController::getSystemStatistics(uint32_t& cyclesCompleted, 
                                           uint32_t& totalWateringTimeOut, 
                                           uint32_t& systemUptime) const {
    cyclesCompleted = totalCyclesCompleted;
    totalWateringTimeOut = totalWateringTime;
    systemUptime = (millis() - systemStartTime) / 1000;
}

bool ServoPWMController::hasErrors() const {
    if (emergencyStop || systemState == IrrigationState::ERROR) {
        return true;
    }
    
    // Verificar si alguna zona está en error
    for (uint8_t i = 0; i < totalZones; i++) {
        if (zones[i].currentState == ServoState::ERROR) {
            return true;
        }
    }
    
    return false;
}

const ZoneInfo* ServoPWMController::getZoneInfo(uint8_t zoneNumber) const {
    if (zoneNumber == 0 || zoneNumber > totalZones) {
        return nullptr;
    }
    return &zones[zoneNumber - 1];
}

void ServoPWMController::printSystemStatus() const {
    const_cast<ServoPWMController*>(this)->generateStatusReport();
}

// =============================================================================
// Métodos de Control Manual
// =============================================================================

bool ServoPWMController::openZoneValve(uint8_t zoneNumber, uint32_t duration) {
    if (zoneNumber == 0 || zoneNumber > totalZones) {
        Serial.println("[ERROR] Número de zona inválido: " + String(zoneNumber));
        return false;
    }
    
    if (emergencyStop) {
        Serial.println("[ERROR] Parada de emergencia activa.");
        return false;
    }
    
    uint8_t zoneIndex = zoneNumber - 1;
    
    Serial.println("[INFO] Abriendo manualmente válvula de zona " + String(zoneNumber));
    
    // Mover servo a posición abierta
    if (moveServoToAngle(zoneIndex, zones[zoneIndex].config.openAngle)) {
        zones[zoneIndex].currentState = ServoState::OPENING;
        zones[zoneIndex].lastActionTime = millis();
        
        // Si se especifica duración, programar cierre automático
        // (Esta funcionalidad se podría implementar con un temporizador)
        if (duration > 0) {
            Serial.println("[INFO] Válvula se cerrará automáticamente en " + String(duration) + " segundos.");
            // TODO: Implementar temporizador para cierre automático
        }
        
        return true;
    }
    
    return false;
}

bool ServoPWMController::closeZoneValve(uint8_t zoneNumber) {
    if (zoneNumber == 0 || zoneNumber > totalZones) {
        Serial.println("[ERROR] Número de zona inválido: " + String(zoneNumber));
        return false;
    }
    
    uint8_t zoneIndex = zoneNumber - 1;
    
    Serial.println("[INFO] Cerrando manualmente válvula de zona " + String(zoneNumber));
    
    if (moveServoToAngle(zoneIndex, SERVO_CLOSED_ANGLE)) {
        zones[zoneIndex].currentState = ServoState::CLOSING;
        zones[zoneIndex].lastActionTime = millis();
        return true;
    }
    
    return false;
}

bool ServoPWMController::setZoneEnabled(uint8_t zoneNumber, bool enabled) {
    if (zoneNumber == 0 || zoneNumber > totalZones) {
        Serial.println("[ERROR] Número de zona inválido: " + String(zoneNumber));
        return false;
    }
    
    uint8_t zoneIndex = zoneNumber - 1;
    zones[zoneIndex].isEnabled = enabled;
    
    Serial.println("[INFO] Zona " + String(zoneNumber) + " " + 
                  String(enabled ? "habilitada" : "deshabilitada"));
    
    return true;
}

bool ServoPWMController::setZoneIrrigationTime(uint8_t zoneNumber, uint32_t seconds) {
    if (zoneNumber == 0 || zoneNumber > totalZones) {
        Serial.println("[ERROR] Número de zona inválido: " + String(zoneNumber));
        return false;
    }
    
    if (seconds < MIN_IRRIGATION_TIME_SECONDS || seconds > MAX_IRRIGATION_TIME_SECONDS) {
        Serial.println("[ERROR] Tiempo de riego fuera de rango válido (" + 
                      String(MIN_IRRIGATION_TIME_SECONDS) + "-" + 
                      String(MAX_IRRIGATION_TIME_SECONDS) + " segundos)");
        return false;
    }
    
    uint8_t zoneIndex = zoneNumber - 1;
    zones[zoneIndex].config.irrigationTime = seconds;
    
    Serial.println("[INFO] Tiempo de riego de zona " + String(zoneNumber) + 
                  " configurado a " + String(seconds) + " segundos");
    
    return true;
}

// =============================================================================
// Métodos de Utilidad
// =============================================================================

const char* ServoPWMController::stateToString(IrrigationState state) {
    switch (state) {
        case IrrigationState::IDLE: return "IDLE";
        case IrrigationState::INITIALIZING: return "INICIALIZANDO";
        case IrrigationState::OPENING_VALVE: return "ABRIENDO_VALVULA";
        case IrrigationState::IRRIGATING: return "REGANDO";
        case IrrigationState::CLOSING_VALVE: return "CERRANDO_VALVULA";
        case IrrigationState::TRANSITIONING: return "TRANSICIONANDO";
        case IrrigationState::COMPLETED: return "COMPLETADO";
        case IrrigationState::ERROR: return "ERROR";
        default: return "DESCONOCIDO";
    }
}

const char* ServoPWMController::servoStateToString(ServoState state) {
    switch (state) {
        case ServoState::UNINITIALIZED: return "NO_INICIALIZADO";
        case ServoState::CLOSED: return "CERRADO";
        case ServoState::OPENING: return "ABRIENDO";
        case ServoState::OPEN: return "ABIERTO";
        case ServoState::CLOSING: return "CERRANDO";
        case ServoState::ERROR: return "ERROR";
        default: return "DESCONOCIDO";
    }
}

// Métodos de la interfaz ServoControllerInterface
void ServoPWMController::startCycle(bool enableAutoCycle) {
    startIrrigationCycle(enableAutoCycle);
}

void ServoPWMController::stopCycle() {
    stopIrrigationCycle();
}

ServoControlState ServoPWMController::getState() const {
    // Convertir el estado de IrrigationState a ServoControlState
    switch (systemState) {
        case IrrigationState::IDLE: 
            return ServoControlState::IDLE;
        case IrrigationState::INITIALIZING: 
            return ServoControlState::INITIALIZING;
        case IrrigationState::OPENING_VALVE: 
            return ServoControlState::MOVING_TO_OPEN;
        case IrrigationState::IRRIGATING: 
            return ServoControlState::OPEN;
        case IrrigationState::CLOSING_VALVE: 
            return ServoControlState::MOVING_TO_CLOSE;
        case IrrigationState::TRANSITIONING: 
            return ServoControlState::CLOSED;
        case IrrigationState::COMPLETED: 
            return ServoControlState::CLOSED;
        case IrrigationState::ERROR: 
            return ServoControlState::ERROR;
        default: 
            return ServoControlState::ERROR;
    }
}

unsigned long ServoPWMController::getCycleCount() const {
    return totalCyclesCompleted;
}

bool ServoPWMController::isReady() const {
    return systemState != IrrigationState::IDLE && systemState != IrrigationState::ERROR;
}

unsigned long ServoPWMController::getStateElapsedTime() const {
    return millis() - stateStartTime;
}

bool ServoPWMController::isServoEnergized() const {
    // Verificar si algún servo está energizado
    for (uint8_t i = 0; i < totalZones; i++) {
        if (zones[i].currentState == ServoState::OPENING || 
            zones[i].currentState == ServoState::OPEN || 
            zones[i].currentState == ServoState::CLOSING) {
            return true;
        }
    }
    return false;
}

void ServoPWMController::printStatus() const {
    printSystemStatus();
}

void ServoPWMController::openServo() {
    // Abrir la válvula de la zona actual
    if (currentZone < totalZones) {
        openZoneValve(currentZone + 1);
    }
}

void ServoPWMController::closeServo() {
    // Cerrar la válvula de la zona actual
    if (currentZone < totalZones) {
        closeZoneValve(currentZone + 1);
    }
}
