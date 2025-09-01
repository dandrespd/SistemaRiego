#ifndef __SERVO_PWM_CONTROLLER_H__
#define __SERVO_PWM_CONTROLLER_H__

/**
 * @file ServoPWMController.h
 * @brief Controlador de servomotores PWM para sistema de riego multi-zona.
 * 
 * Esta clase implementa un sistema de control secuencial de servomotores que actúan
 * como válvulas inteligentes para dirigir el flujo de agua hacia diferentes zonas
 * de riego. El diseño modular permite escalar fácilmente el número de zonas y
 * personalizar los parámetros de riego de cada una.
 * 
 * CONCEPTOS CLAVE:
 * - PWM (Modulación por Ancho de Pulso): Técnica para controlar servomotores
 *   variando la duración del pulso en una señal de frecuencia fija.
 * - Riego Secuencial: Las zonas se riegan una por vez para mantener presión
 *   adecuada y evitar sobrecargar la bomba de agua.
 * - Control No Bloqueante: El sistema puede realizar otras tareas mientras
 *   controla los servomotores.
 * 
 * FUNCIONAMIENTO:
 * 1. Inicialización: Configura todos los servomotores en posición cerrada
 * 2. Secuencia de Riego: Para cada zona habilitada:
 *    a) Abre la válvula (mueve servo a posición configurada)
 *    b) Mantiene riego durante el tiempo especificado
 *    c) Cierra la válvula (retorna servo a posición cerrada)
 *    d) Pausa de transición antes de la siguiente zona
 * 3. Repetición: El ciclo se puede configurar para repetirse automáticamente
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0
 * @date 2025
 * @dependencies SET_PIN.h, SERVO_CONFIG.h, ESP32 Arduino Framework
*/

// =============================================================================
// Includes necesarios
// =============================================================================
#include <Arduino.h>
#include <stdint.h>
#include "SET_PIN.h"
#include "SERVO_CONFIG.h"
#include "ServoControllerInterface.h"

// =============================================================================
// Enumeraciones para Estados del Sistema
// =============================================================================

/**
 * @brief Estados posibles del sistema de riego multi-zona.
 * 
 * Esta enumeración define todos los estados posibles del controlador,
 * permitiendo un control preciso del flujo de ejecución y facilitando
 * el debugging y monitoreo del sistema.
 */
enum class IrrigationState {
    IDLE,                    // Sistema inactivo, esperando comandos
    INITIALIZING,           // Inicializando servomotores y configuración
    OPENING_VALVE,          // Abriendo válvula de la zona actual
    IRRIGATING,             // Regando zona actual (válvula abierta)
    CLOSING_VALVE,          // Cerrando válvula de la zona actual
    TRANSITIONING,          // Pausa entre zonas para estabilización
    COMPLETED,              // Ciclo de riego completado
    ERROR                   // Error en el sistema (requiere intervención)
};

/**
 * @brief Estados posibles de un servomotor individual.
 * 
 * Cada servomotor puede estar en diferentes estados que nos ayudan
 * a monitorear su funcionamiento y detectar problemas.
 */
enum class ServoState {
    UNINITIALIZED,          // Servo no inicializado
    CLOSED,                 // Servo en posición cerrada (válvula cerrada)
    OPENING,                // Servo moviéndose hacia posición abierta
    OPEN,                   // Servo en posición abierta (válvula abierta)
    CLOSING,                // Servo moviéndose hacia posición cerrada
    ERROR                   // Error en el servo (no responde o mal funcionamiento)
};

// =============================================================================
// Estructura de Datos para Información de Zona
// =============================================================================

/**
 * @brief Información de estado y configuración de una zona de riego.
 * 
 * Esta estructura contiene toda la información necesaria para monitorear
 * y controlar una zona individual del sistema de riego.
 */
struct ZoneInfo {
    uint8_t zoneNumber;           // Número de zona (1, 2, 3, etc.)
    uint8_t servoPin;             // Pin GPIO del servomotor
    uint8_t pwmChannel;           // Canal PWM asignado (ESP32 tiene 16 canales)
    ServoState currentState;      // Estado actual del servomotor
    uint32_t lastActionTime;      // Timestamp de la última acción realizada
    uint32_t totalIrrigationTime; // Tiempo total regado en esta sesión
    bool isEnabled;               // Zona habilitada para riego
    ServoZoneConfig config;            // Configuración específica de la zona
    uint8_t retryCount;           // Número de intentos de reposicionamiento
};

// =============================================================================
// Clase Principal: ServoPWMController
// =============================================================================

/**
 * @class ServoPWMController
 * @brief Controlador principal del sistema de riego multi-zona con servomotores.
 * 
 * Esta clase encapsula toda la lógica necesaria para controlar múltiples
 * servomotores de forma secuencial, implementando un sistema de riego
 * inteligente y eficiente.
 * 
 * CARACTERÍSTICAS PRINCIPALES:
 * - Control PWM nativo del ESP32 (sin librerías externas)
 * - Operación no bloqueante (compatible con otras tareas)
 * - Configuración flexible por zona
 * - Sistema robusto de manejo de errores
 * - Monitoreo y logging detallado
 * - Escalable a cualquier número de zonas
 * 
 * PATRÓN DE DISEÑO:
 * Sigue el patrón \"Estado\" (State Pattern) donde el comportamiento
 * del objeto cambia según su estado interno, facilitando el mantenimiento
 * y la extensión del código.
 */
class ServoPWMController : public ServoControllerInterface {

private:
    // =========================================================================
    // Atributos Privados - Estado Interno del Sistema
    // =========================================================================
    
    ZoneInfo* zones;                    // Array dinámico de información de zonas
    uint8_t totalZones;                 // Número total de zonas configuradas
    uint8_t currentZone;                // Zona que se está procesando actualmente
    IrrigationState systemState;        // Estado actual del sistema completo
    
    unsigned long stateStartTime;       // Tiempo de inicio del estado actual
    unsigned long lastStatusReport;     // Último reporte de estado enviado
    
    bool autoCycle;                     // Repetir ciclo automáticamente
    bool emergencyStop;                 // Parada de emergencia activada
    
    // Estadísticas del sistema
    uint32_t totalCyclesCompleted;      // Ciclos completos de riego realizados
    uint32_t totalWateringTime;         // Tiempo total de riego acumulado
    unsigned long systemStartTime;      // Tiempo de inicio del sistema
    
    // =========================================================================
    // Métodos Privados - Funciones Internas de la Clase
    // =========================================================================
    
    /**
     * @brief Inicializa los canales PWM para todos los servomotores.
     * 
     * Configura el hardware PWM del ESP32 asignando un canal único a cada
     * servomotor y estableciendo la frecuencia y resolución adecuadas.
     * 
     * @return true si la inicialización fue exitosa, false en caso de error
     */
    bool initializePWMChannels();
    
    /**
     * @brief Configura un servomotor específico en su posición inicial.
     * 
     * @param zoneIndex Índice de la zona (0-based)
     * @return true si la configuración fue exitosa, false en caso de error
     */
    bool setupServo(uint8_t zoneIndex);
    
    /**
     * @brief Mueve un servomotor a un ángulo específico de forma gradual.
     * 
     * Implementa movimiento suave del servo para evitar cambios bruscos
     * que podrían causar golpes de ariete en el sistema hidráulico.
     * 
     * @param zoneIndex Índice de la zona
     * @param targetAngle Ángulo objetivo (0-180 grados)
     * @return true si el movimiento se completó correctamente
     */
    bool moveServoToAngle(uint8_t zoneIndex, uint8_t targetAngle);
    
    /**
     * @brief Convierte un ángulo en grados a valor PWM correspondiente.
     * 
     * Los servomotores requieren pulsos de 1-2ms para posiciones 0-180°.
     * Esta función realiza la conversión matemática considerando la
     * frecuencia y resolución configuradas.
     * 
     * @param angle Ángulo en grados (0-180)
     * @return Valor PWM correspondiente
     */
    uint32_t angleToHaltValue(uint8_t angle);
    
    /**
     * @brief Verifica si un servomotor ha alcanzado su posición objetivo.
     * 
     * @param zoneIndex Índice de la zona
     * @return true si el servo está en posición, false en caso contrario
     */
    bool isServoInPosition(uint8_t zoneIndex);
    
    /**
     * @brief Maneja errores de servomotores y aplica estrategias de recuperación.
     * 
     * @param zoneIndex Índice de la zona con error
     * @param errorType Tipo de error detectado
     * @return true si el error fue resuelto, false si persiste
     */
    bool handleServoError(uint8_t zoneIndex, const char* errorType);
    
    /**
     * @brief Actualiza el estado del sistema basado en condiciones actuales.
     * 
     * Esta función implementa la máquina de estados del sistema,
     * determinando transiciones y acciones a realizar.
     */
    void updateSystemState();
    
    /**
     * @brief Valida la configuración del sistema antes de iniciar operaciones.
     * 
     * @return true si la configuración es válida, false en caso contrario
     */
    bool validateConfiguration();
    
    /**
     * @brief Genera reportes detallados del estado del sistema.
     * 
     * Envía información de diagnóstico al puerto serie para monitoreo
     * y debugging del sistema.
     */
    void generateStatusReport() const;
    
    /**
     * @brief Inicializa las zonas del controlador.
     * 
     * Método auxiliar para centralizar la inicialización de zonas,
     * utilizado por constructores y operadores de asignación.
     * 
     * @param numZones Número de zonas a inicializar
     */
    void initializeZones(uint8_t numZones);
    
    /**
     * @brief Limpia los recursos del controlador.
     * 
     * Método auxiliar para centralizar la limpieza de recursos,
     * utilizado por destructor y operadores de asignación.
     */
    void cleanup();

    // Métodos de manejo de estados (deben estar declarados)
    void handleInitializingState();
    void handleOpeningValveState();
    void handleIrrigatingState();
    void handleClosingValveState();
    void handleTransitioningState();
    void handleCompletedState();
    void handleErrorState();

public:
    // =========================================================================
    // Constructor y Destructor
    // =========================================================================
    
    /**
     * @brief Constructor de la clase ServoPWMController.
     * 
     * Inicializa el controlador con el número de zonas especificado.
     * No realiza configuración de hardware hasta que se llame a init().
     * 
     * @param numZones Número de zonas de riego a controlar
     */
    explicit ServoPWMController(uint8_t numZones = NUM_SERVOS);
    
    /**
     * @brief Constructor de copia.
     * 
     * Crea una copia profunda del controlador, incluyendo toda la configuración
     * y estado de las zonas. Útil para crear respaldos o múltiples instancias.
     * 
     * @param other Controlador a copiar
     */
    ServoPWMController(const ServoPWMController& other);
    
    /**
     * @brief Operador de asignación.
     * 
     * Asigna el contenido de otro controlador a este, realizando una copia profunda.
     * Maneja correctamente la limpieza de recursos existentes.
     * 
     * @param other Controlador a asignar
     * @return Referencia a este controlador
     */
    ServoPWMController& operator=(const ServoPWMController& other);
    
    /**
     * @brief Constructor de movimiento (C++11).
     * 
     * Transfiere recursos de otro controlador de forma eficiente.
     * El controlador original queda en estado válido pero vacío.
     * 
     * @param other Controlador a mover (rvalue reference)
     */
    ServoPWMController(ServoPWMController&& other) noexcept;
    
    /**
     * @brief Operador de asignación de movimiento (C++11).
     * 
     * Transfiere recursos de forma eficiente, limpiando los recursos
     * existentes de este controlador.
     * 
     * @param other Controlador a mover (rvalue reference)
     * @return Referencia a este controlador
     */
    ServoPWMController& operator=(ServoPWMController&& other) noexcept;
    
    /**
     * @brief Destructor de la clase.
     * 
     * Limpia recursos y asegura que todos los servomotores estén
     * en posición segura antes de la destrucción del objeto.
     */
    ~ServoPWMController();

    // =========================================================================
    // Métodos Públicos - Interfaz Principal de la Clase
    // =========================================================================
    
    /**
     * @brief Inicializa el sistema de control de servomotores.
     * 
     * Configura el hardware PWM, inicializa todas las zonas y verifica
     * que el sistema esté listo para operar. Debe llamarse antes de
     * cualquier otra operación.
     * 
     * @return true si la inicialización fue exitosa, false en caso de error
     */
    bool init();
    
    /**
     * @brief Inicia un ciclo completo de riego en todas las zonas habilitadas.
     * 
     * Comienza la secuencia de riego que procesará cada zona según
     * su configuración individual. La operación es no bloqueante.
     * 
     * @param enableAutoCycle Si true, repite el ciclo automáticamente
     * @return true si el ciclo se inició correctamente
     */
    bool startIrrigationCycle(bool enableAutoCycle = false);
    
    /**
     * @brief Inicia el ciclo de movimiento del servo.
     * 
     * Este método es parte de la interfaz ServoControllerInterface.
     * 
     * @param enableAutoCycle Si true, repite el ciclo automáticamente
     */
    void startCycle(bool enableAutoCycle = false) override;
    
    /**
     * @brief Detiene el ciclo actual.
     */
    void stopCycle() override;
    
    /**
     * @brief Función principal de actualización (llamar en loop).
     */
    void update() override;
    
    /**
     * @brief Abre el servo manualmente.
     */
    void openServo() override;
    
    /**
     * @brief Cierra el servo manualmente.
     */
    void closeServo() override;
    
    /**
     * @brief Obtiene el estado actual del servo.
     * @return Estado actual.
     */
    ServoControlState getState() const override;
    
    /**
     * @brief Obtiene el número de ciclos completados.
     * @return Número de ciclos.
     */
    unsigned long getCycleCount() const override;
    
    /**
     * @brief Verifica si el servo está inicializado.
     * @return true si está inicializado.
     */
    bool isReady() const override;
    
    /**
     * @brief Obtiene el tiempo transcurrido en el estado actual.
     * @return Tiempo en milisegundos.
     */
    unsigned long getStateElapsedTime() const override;
    
    /**
     * @brief Verifica si el servo está energizado.
     * @return true si está energizado.
     */
    bool isServoEnergized() const override;
    
    /**
     * @brief Imprime información del estado actual.
     */
    void printStatus() const override;
    
    /**
     * @brief Detiene el ciclo de riego actual de forma segura.
     * 
     * Cierra todas las válvulas abiertas y retorna el sistema a estado idle.
     * Es una operación segura que puede llamarse en cualquier momento.
     */
    void stopIrrigationCycle();
    
    /**
     * @brief Activa parada de emergencia del sistema.
     * 
     * Cierra inmediatamente todas las válvulas y detiene todas las operaciones.
     * Requiere llamar a resetEmergencyStop() para reanudar operaciones.
     */
    void emergencyStopAll();
    
    /**
     * @brief Desactiva la parada de emergencia.
     * 
     * Permite reanudar operaciones normales después de una parada de emergencia.
     * Requiere reinicialización del sistema con init().
     */
    void resetEmergencyStop();
    
    
    /**
     * @brief Abre manualmente la válvula de una zona específica.
     * 
     * Permite control manual de zonas individuales para testing
     * o situaciones especiales.
     * 
     * @param zoneNumber Número de zona (1-based)
     * @param duration Duración en segundos (0 = indefinido)
     * @return true si la operación fue exitosa
     */
    bool openZoneValve(uint8_t zoneNumber, uint32_t duration = 0);
    
    /**
     * @brief Cierra manualmente la válvula de una zona específica.
     * 
     * @param zoneNumber Número de zona (1-based)
     * @return true si la operación fue exitosa
     */
    bool closeZoneValve(uint8_t zoneNumber);
    
    /**
     * @brief Habilita o deshabilita una zona específica.
     * 
     * Las zonas deshabilitadas se omiten durante los ciclos automáticos.
     * 
     * @param zoneNumber Número de zona (1-based)
     * @param enabled true para habilitar, false para deshabilitar
     * @return true si la operación fue exitosa
     */
    bool setZoneEnabled(uint8_t zoneNumber, bool enabled);
    
    /**
     * @brief Configura el tiempo de riego para una zona específica.
     * 
     * @param zoneNumber Número de zona (1-based)
     * @param seconds Tiempo de riego en segundos
     * @return true si la configuración fue exitosa
     */
    bool setZoneIrrigationTime(uint8_t zoneNumber, uint32_t seconds);
    
    // =========================================================================
    // Métodos de Consulta de Estado
    // =========================================================================
    
    /**
     * @brief Obtiene el estado actual del sistema.
     * 
     * @return Estado actual del sistema de riego
     */
    IrrigationState getCurrentState() const;
    
    /**
     * @brief Obtiene el estado de una zona específica.
     * 
     * @param zoneNumber Número de zona (1-based)
     * @return Estado del servomotor de la zona
     */
    ServoState getZoneState(uint8_t zoneNumber) const;
    
    /**
     * @brief Verifica si el sistema está actualmente regando.
     * 
     * @return true si hay al menos una zona regando activamente
     */
    bool isIrrigating() const;
    
    /**
     * @brief Obtiene el número de la zona que se está regando actualmente.
     * 
     * @return Número de zona (1-based), 0 si ninguna zona está activa
     */
    uint8_t getCurrentActiveZone() const;
    
    /**
     * @brief Obtiene el tiempo restante de riego para la zona actual.
     * 
     * @return Tiempo restante en segundos, 0 si no hay riego activo
     */
    uint32_t getRemainingIrrigationTime() const;
    
    /**
     * @brief Obtiene estadísticas del sistema de riego.
     * 
     * @param cyclesCompleted Referencia donde almacenar ciclos completados
     * @param totalWateringTime Referencia donde almacenar tiempo total de riego
     * @param systemUptime Referencia donde almacenar tiempo de funcionamiento
     */
    void getSystemStatistics(uint32_t& cyclesCompleted, uint32_t& totalWateringTime, uint32_t& systemUptime) const;
    
    /**
     * @brief Verifica si el sistema está en estado de error.
     * 
     * @return true si hay errores que requieren atención
     */
    bool hasErrors() const;
    
    /**
     * @brief Obtiene información detallada de una zona específica.
     * 
     * @param zoneNumber Número de zona (1-based)
     * @return Puntero a información de zona, nullptr si la zona no existe
     */
    const ZoneInfo* getZoneInfo(uint8_t zoneNumber) const;
    
    /**
     * @brief Genera un reporte completo del estado del sistema.
     * 
     * Envía información detallada al puerto serie para diagnóstico.
     */
    void printSystemStatus() const;
    
    /**
     * @brief Convierte un estado del sistema a texto legible.
     * 
     * @param state Estado a convertir
     * @return Cadena de texto descriptiva del estado
     */
    static const char* stateToString(IrrigationState state);
    
    /**
     * @brief Convierte un estado de servo a texto legible.
     * 
     * @param state Estado del servo a convertir
     * @return Cadena de texto descriptiva del estado
     */
    static const char* servoStateToString(ServoState state);
};

#endif // __SERVO_PWM_CONTROLLER_H__
