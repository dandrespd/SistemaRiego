#ifndef __LOGGER_H__
#define __LOGGER_H__

/**
 * @file Logger.h
 * @brief Sistema de logging unificado para el sistema de riego inteligente.
 * 
 * Implementa un sistema centralizado de logging con múltiples niveles y
 * capacidad de registro en diferentes destinos (serial, archivo, red).
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0
 * @date 2025
 */

#include <Arduino.h>
#include <stdint.h>
#include <WString.h>

/**
 * @brief Niveles de logging disponibles.
 */
enum class LogLevel {
    ERROR = 0,
    WARNING = 1,
    INFO = 2,
    DEBUG = 3,
    VERBOSE = 4
};

/**
 * @class Logger
 * @brief Clase singleton para manejo centralizado de logs.
 */
class Logger {
private:
    static Logger* instance;
    LogLevel currentLevel;
    bool logToSerial;
    bool logToFile;
    bool logToWeb;
    String logBuffer;
    uint32_t lastFlushTime;
    uint32_t bufferSize;

    // Constructor privado para patrón singleton
    Logger();

public:
    // Eliminar copia y asignación
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Obtiene la instancia singleton del Logger.
     */
    static Logger& getInstance();

    /**
     * @brief Inicializa el sistema de logging.
     */
    void initialize(LogLevel level = LogLevel::INFO, bool serial = true, bool file = false, bool web = false);

    /**
     * @brief Registra un mensaje de error.
     */
    void error(const String& message);

    /**
     * @brief Registra un mensaje de advertencia.
     */
    void warning(const String& message);

    /**
     * @brief Registra un mensaje informativo.
     */
    void info(const String& message);

    /**
     * @brief Registra un mensaje de depuración.
     */
    void debug(const String& message);

    /**
     * @brief Registra un mensaje verbose.
     */
    void verbose(const String& message);

    /**
     * @brief Establece el nivel de logging.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Obtiene el nivel de logging actual.
     */
    LogLevel getLevel() const;

    /**
     * @brief Habilita/deshabilita logging a serial.
     */
    void setSerialLogging(bool enabled);

    /**
     * @brief Habilita/deshabilita logging a archivo.
     */
    void setFileLogging(bool enabled);

    /**
     * @brief Habilita/deshabilita logging a web.
     */
    void setWebLogging(bool enabled);

    /**
     * @brief Fuerza el vaciado del buffer de logs.
     */
    void flush();

private:
    /**
     * @brief Método interno para registrar mensajes.
     */
    void log(LogLevel level, const String& message);

    /**
     * @brief Formatea el timestamp para los logs.
     */
    String formatTimestamp();

    /**
     * @brief Escribe el log al serial.
     */
    void writeToSerial(const String& formattedMessage);

    /**
     * @brief Escribe el log a archivo.
     */
    void writeToFile(const String& formattedMessage);

    /**
     * @brief Escribe el log a la interfaz web.
     */
    void writeToWeb(const String& formattedMessage);

    /**
     * @brief Obtiene el nombre del nivel de log.
     */
    String getLevelName(LogLevel level) const;
};

// Macros para facilitar el uso del logger
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_VERBOSE(msg) Logger::getInstance().verbose(msg)

#endif // __LOGGER_H__
