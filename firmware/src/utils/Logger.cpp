/**
 * @file Logger.cpp
 * @brief Implementación del sistema de logging unificado.
 */

#include "Logger.h"
#include <SPIFFS.h>
#include <Arduino.h>

// Instancia singleton
Logger* Logger::instance = nullptr;

Logger::Logger()
    : currentLevel(LogLevel::INFO)
    , logToSerial(true)
    , logToFile(false)
    , logToWeb(false)
    , lastFlushTime(0)
    , bufferSize(0)
{
}

Logger& Logger::getInstance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return *instance;
}

void Logger::initialize(LogLevel level, bool serial, bool file, bool web) {
    currentLevel = level;
    logToSerial = serial;
    logToFile = file;
    logToWeb = web;
    
    if (logToFile) {
        // Inicializar SPIFFS para logging a archivo
        if (!SPIFFS.begin(true)) {
            Serial.println("[LOGGER ERROR] No se pudo inicializar SPIFFS para logging");
            logToFile = false;
        }
    }
    
    info("Sistema de logging inicializado. Nivel: " + getLevelName(level));
}

void Logger::error(const String& message) {
    log(LogLevel::ERROR, message);
}

void Logger::warning(const String& message) {
    log(LogLevel::WARNING, message);
}

void Logger::info(const String& message) {
    log(LogLevel::INFO, message);
}

void Logger::debug(const String& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::verbose(const String& message) {
    log(LogLevel::VERBOSE, message);
}

void Logger::setLevel(LogLevel level) {
    currentLevel = level;
    info("Nivel de logging cambiado a: " + getLevelName(level));
}

LogLevel Logger::getLevel() const {
    return currentLevel;
}

void Logger::setSerialLogging(bool enabled) {
    logToSerial = enabled;
    info("Logging a serial " + String(enabled ? "habilitado" : "deshabilitado"));
}

void Logger::setFileLogging(bool enabled) {
    logToFile = enabled;
    if (enabled && !SPIFFS.begin(true)) {
        Serial.println("[LOGGER ERROR] No se pudo inicializar SPIFFS para logging");
        logToFile = false;
    }
    info("Logging a archivo " + String(enabled ? "habilitado" : "deshabilitado"));
}

void Logger::setWebLogging(bool enabled) {
    logToWeb = enabled;
    info("Logging a web " + String(enabled ? "habilitado" : "deshabilitado"));
}

void Logger::flush() {
    if (logBuffer.length() > 0 && logToFile) {
        File logFile = SPIFFS.open("/system.log", "a");
        if (logFile) {
            logFile.println(logBuffer);
            logFile.close();
            logBuffer = "";
            bufferSize = 0;
        }
    }
    lastFlushTime = millis();
}

void Logger::log(LogLevel level, const String& message) {
    // Verificar si el nivel de log está habilitado
    if (static_cast<int>(level) > static_cast<int>(currentLevel)) {
        return;
    }
    
    String formattedMessage = formatTimestamp() + " [" + getLevelName(level) + "] " + message;
    
    // Escribir a los destinos configurados
    if (logToSerial) {
        writeToSerial(formattedMessage);
    }
    
    if (logToFile) {
        writeToFile(formattedMessage);
    }
    
    if (logToWeb) {
        writeToWeb(formattedMessage);
    }
    
    // Manejar buffer para logging a archivo
    if (logToFile) {
        logBuffer += formattedMessage + "\n";
        bufferSize += formattedMessage.length() + 1;
        
        // Vaciar buffer si excede el tamaño o ha pasado el tiempo
        if (bufferSize > 1024 || millis() - lastFlushTime > 5000) {
            flush();
        }
    }
}

String Logger::formatTimestamp() {
    unsigned long millisNow = millis();
    unsigned long seconds = millisNow / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    char timestamp[16];
    snprintf(timestamp, sizeof(timestamp), "[%02lu:%02lu:%02lu.%03lu]", 
             hours % 24, minutes % 60, seconds % 60, millisNow % 1000);
    
    return String(timestamp);
}

void Logger::writeToSerial(const String& formattedMessage) {
    Serial.println(formattedMessage);
}

void Logger::writeToFile(const String& formattedMessage) {
    // El logging a archivo se maneja mediante el buffer y flush()
    // para evitar abrir/cerrar el archivo constantemente
}

void Logger::writeToWeb(const String& formattedMessage) {
    // Implementación pendiente - se integrará con WebSocketManager
    // Por ahora solo se escribe a serial como placeholder
    Serial.println("[WEB] " + formattedMessage);
}

String Logger::getLevelName(LogLevel level) const {
    switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::INFO: return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::VERBOSE: return "VERBOSE";
        default: return "UNKNOWN";
    }
}
