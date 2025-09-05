#ifndef __SERVO_CONTROLLER_INTERFACE_H__
#define __SERVO_CONTROLLER_INTERFACE_H__

/**
 * @file ServoControllerInterface.h
 * @brief Interfaz común para controladores de servomotores.
 * 
 * Esta interfaz define un contrato común para todos los controladores de servomotores,
 * permitiendo la interoperabilidad entre diferentes implementaciones y patrones de control.
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0
 * @date 2025
 */

/**
 * @enum ServoControlState
 * @brief Estados posibles de un controlador de servo.
 */
enum class ServoControlState {
    IDLE,           // Servo en reposo
    INITIALIZING,   // Inicializando servo
    MOVING_TO_OPEN, // Moviéndose a posición abierta
    OPEN,           // En posición abierta
    MOVING_TO_CLOSE, // Moviéndose a posición cerrada
    CLOSED,         // En posición cerrada
    ERROR           // Error en el servo
};

/**
 * @class ServoControllerInterface
 * @brief Interfaz base para todos los controladores de servomotores.
 * 
 * Esta interfaz proporciona un contrato común que todas las implementaciones
 * de controladores de servo deben cumplir, permitiendo el uso polimórfico
 * de diferentes tipos de controladores.
 */
class ServoControllerInterface {
public:
    /**
     * @brief Destructor virtual para permitir eliminación polimórfica.
     */
    virtual ~ServoControllerInterface() = default;

    /**
     * @brief Inicializa el servo.
     * @return true si la inicialización fue exitosa.
     */
    virtual bool init() = 0;

    /**
     * @brief Inicia el ciclo de movimiento del servo.
     * @param enableAutoCycle Si true, repite el ciclo automáticamente.
     */
    virtual void startCycle(bool enableAutoCycle = false) = 0;

    /**
     * @brief Detiene el ciclo actual.
     */
    virtual void stopCycle() = 0;

    /**
     * @brief Función principal de actualización (llamar en loop).
     */
    virtual void update() = 0;

    /**
     * @brief Abre el servo manualmente.
     */
    virtual void openServo() = 0;

    /**
     * @brief Cierra el servo manualmente.
     */
    virtual void closeServo() = 0;

    /**
     * @brief Obtiene el estado actual del servo.
     * @return Estado actual.
     */
    virtual ServoControlState getState() const = 0;

    /**
     * @brief Obtiene el número de ciclos completados.
     * @return Número de ciclos.
     */
    virtual unsigned long getCycleCount() const = 0;

    /**
     * @brief Verifica si el servo está inicializado.
     * @return true si está inicializado.
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Obtiene el tiempo transcurrido en el estado actual.
     * @return Tiempo en milisegundos.
     */
    virtual unsigned long getStateElapsedTime() const = 0;

    /**
     * @brief Verifica si el servo está energizado.
     * @return true si está energizado.
     */
    virtual bool isServoEnergized() const = 0;

    /**
     * @brief Convierte el estado a string.
     * @param state Estado a convertir.
     * @return String descriptivo.
     */
    static const char* stateToString(ServoControlState state) {
        switch (state) {
            case ServoControlState::IDLE: return "IDLE";
            case ServoControlState::INITIALIZING: return "INITIALIZING";
            case ServoControlState::MOVING_TO_OPEN: return "MOVING_TO_OPEN";
            case ServoControlState::OPEN: return "OPEN";
            case ServoControlState::MOVING_TO_CLOSE: return "MOVING_TO_CLOSE";
            case ServoControlState::CLOSED: return "CLOSED";
            case ServoControlState::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    /**
     * @brief Imprime información del estado actual.
     */
    virtual void printStatus() const = 0;
};

#endif // __SERVO_CONTROLLER_INTERFACE_H__
