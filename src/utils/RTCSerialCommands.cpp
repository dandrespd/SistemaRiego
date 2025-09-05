/**
 * @file RTCSerialCommands.cpp
 * @brief Comandos seriales para configuración y gestión del RTC DS1302
 * 
 * Este archivo implementa comandos seriales que permiten configurar
 * y consultar el RTC a través del monitor serial.
 * 
 * Comandos disponibles:
 * - rtc_get: Obtener fecha y hora actual
 * - rtc_set YYYY-MM-DD HH:MM:SS: Establecer fecha y hora
 * - rtc_status: Obtener estado del RTC
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0
 * @date 2025
 */

#include <Arduino.h>
#include "../include/ProjectConfig.h"
#include "drivers/RTC_DS1302.h"
#include "utils/Logger.h"

class RTCSerialCommands {
private:
    RTC_DS1302* rtcInstance;
    String inputBuffer;
    
public:
    RTCSerialCommands(RTC_DS1302* rtc) : rtcInstance(rtc), inputBuffer("") {}
    
    /**
     * @brief Procesar comandos seriales entrantes
     * Debe llamarse regularmente en el loop principal
     */
    void processCommands() {
        while (Serial.available()) {
            char inChar = (char)Serial.read();
            
            if (inChar == '\n' || inChar == '\r') {
                if (inputBuffer.length() > 0) {
                    processCommand(inputBuffer);
                    inputBuffer = "";
                }
            } else {
                inputBuffer += inChar;
            }
        }
    }
    
private:
    /**
     * @brief Procesar un comando completo
     * @param command Comando a procesar
     */
    void processCommand(String command) {
        command.trim();
        command.toLowerCase();
        
        LOG_INFO("[RTC_SERIAL] Comando recibido: " + command);
        
        if (command == "rtc_get" || command == "get_time") {
            handleGetDateTime();
        } else if (command.startsWith("rtc_set ") || command.startsWith("set_time ")) {
            String dateTimeStr = extractDateTime(command);
            handleSetDateTime(dateTimeStr);
        } else if (command == "rtc_status") {
            handleGetStatus();
        } else if (command == "rtc_help" || command == "help") {
            printHelp();
        } else if (command.startsWith("rtc_")) {
            Serial.println("[RTC_SERIAL] ERROR: Comando RTC no reconocido. Usa 'rtc_help' para ver comandos disponibles.");
        }
        // Si no empieza con rtc_, no es un comando RTC, ignorar silenciosamente
    }
    
    /**
     * @brief Manejar comando de obtener fecha/hora
     */
    void handleGetDateTime() {
        Serial.println("\n=== FECHA Y HORA ACTUAL ===");
        
        if (!rtcInstance) {
            Serial.println("[RTC_SERIAL] ERROR: Instancia RTC no disponible");
            return;
        }
        
        if (!rtcInstance->isInitialized()) {
            Serial.println("[RTC_SERIAL] ERROR: RTC no inicializado");
            return;
        }
        
        RTC_DS1302::DateTime dt = rtcInstance->getDateTime();
        if (!dt.isValid()) {
            Serial.println("[RTC_SERIAL] ERROR: No se pudo leer la fecha/hora del RTC");
            Serial.println("Error: " + rtcErrorToString(rtcInstance->getLastError()));
            return;
        }
        
        // Mostrar fecha/hora formateada
        String formattedDateTime = formatDateTime(dt);
        Serial.println("Fecha/Hora: " + formattedDateTime);
        Serial.println("Detalles:");
        Serial.println("  Año: " + String(2000 + dt.year));
        Serial.println("  Mes: " + String(dt.month));
        Serial.println("  Día: " + String(dt.day));
        Serial.println("  Día de la semana: " + dayOfWeekToString(dt.dayOfWeek));
        Serial.println("  Hora: " + String(dt.hour));
        Serial.println("  Minuto: " + String(dt.minute));
        Serial.println("  Segundo: " + String(dt.second));
        Serial.println("===============================\n");
    }
    
    /**
     * @brief Manejar comando de establecer fecha/hora
     * @param dateTimeStr String con fecha/hora en formato "YYYY-MM-DD HH:MM:SS"
     */
    void handleSetDateTime(String dateTimeStr) {
        Serial.println("\n=== CONFIGURAR FECHA Y HORA ===");
        
        if (!rtcInstance) {
            Serial.println("[RTC_SERIAL] ERROR: Instancia RTC no disponible");
            return;
        }
        
        if (!rtcInstance->isInitialized()) {
            Serial.println("[RTC_SERIAL] ERROR: RTC no inicializado");
            return;
        }
        
        if (dateTimeStr.length() == 0) {
            Serial.println("[RTC_SERIAL] ERROR: Formato requerido: rtc_set YYYY-MM-DD HH:MM:SS");
            Serial.println("Ejemplo: rtc_set 2025-01-15 14:30:00");
            return;
        }
        
        RTC_DS1302::DateTime dt;
        if (!parseDateTime(dateTimeStr, dt)) {
            Serial.println("[RTC_SERIAL] ERROR: Formato de fecha/hora inválido");
            Serial.println("Formato requerido: YYYY-MM-DD HH:MM:SS");
            Serial.println("Ejemplo: 2025-01-15 14:30:00");
            return;
        }
        
        Serial.println("Configurando fecha/hora: " + formatDateTime(dt));
        
        if (rtcInstance->setDateTime(dt)) {
            Serial.println("[RTC_SERIAL] ✅ Fecha/hora configurada exitosamente");
            
            // Verificar configuración leyendo de nuevo
            delay(100);
            RTC_DS1302::DateTime readBack = rtcInstance->getDateTime();
            if (readBack.isValid()) {
                Serial.println("Verificación: " + formatDateTime(readBack));
            }
        } else {
            Serial.println("[RTC_SERIAL] ❌ Error al configurar fecha/hora");
            Serial.println("Error: " + rtcErrorToString(rtcInstance->getLastError()));
        }
        Serial.println("===============================\n");
    }
    
    /**
     * @brief Manejar comando de estado del RTC
     */
    void handleGetStatus() {
        Serial.println("\n=== ESTADO DEL RTC ===");
        
        if (!rtcInstance) {
            Serial.println("[RTC_SERIAL] ERROR: Instancia RTC no disponible");
            return;
        }
        
        Serial.println("Estado de inicialización: " + String(rtcInstance->isInitialized() ? "✅ Inicializado" : "❌ No inicializado"));
        Serial.println("Estado de funcionamiento: " + String(rtcInstance->isRunning() ? "✅ Funcionando" : "❌ Detenido"));
        Serial.println("Último error: " + rtcErrorToString(rtcInstance->getLastError()));
        
        // Información de pines
        Serial.println("\nConfiguración de pines:");
        Serial.println("  RST: " + String(globalConfig.rtc_rst));
        Serial.println("  SCLK: " + String(globalConfig.rtc_sclk));
        Serial.println("  IO: " + String(globalConfig.rtc_io));
        
        // Información adicional si está inicializado
        if (rtcInstance->isInitialized()) {
            RTC_DS1302::DateTime dt = rtcInstance->getDateTime();
            if (dt.isValid()) {
                Serial.println("\nFecha/hora actual: " + formatDateTime(dt));
            } else {
                Serial.println("\n❌ Error al leer fecha/hora actual");
            }
        }
        
        Serial.println("======================\n");
    }
    
    /**
     * @brief Mostrar ayuda de comandos RTC
     */
    void printHelp() {
        Serial.println("\n=== COMANDOS RTC DISPONIBLES ===");
        Serial.println("rtc_get          - Obtener fecha y hora actual");
        Serial.println("rtc_set <fecha>  - Establecer fecha y hora");
        Serial.println("                   Formato: YYYY-MM-DD HH:MM:SS");
        Serial.println("                   Ejemplo: rtc_set 2025-01-15 14:30:00");
        Serial.println("rtc_status       - Mostrar estado del RTC");
        Serial.println("rtc_help         - Mostrar esta ayuda");
        Serial.println("\nALIAS ALTERNATIVOS:");
        Serial.println("get_time         - Mismo que rtc_get");
        Serial.println("set_time <fecha> - Mismo que rtc_set");
        Serial.println("help             - Mismo que rtc_help");
        Serial.println("================================\n");
    }
    
    /**
     * @brief Extraer fecha/hora de un comando
     * @param command Comando completo
     * @return String con la parte de fecha/hora
     */
    String extractDateTime(String command) {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex == -1) return "";
        return command.substring(spaceIndex + 1);
    }
    
    /**
     * @brief Formatear DateTime a string legible
     * @param dt DateTime a formatear
     * @return String formateado
     */
    String formatDateTime(const RTC_DS1302::DateTime& dt) {
        char buffer[32];
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
                2000 + dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        return String(buffer);
    }
    
    /**
     * @brief Parsear string de fecha/hora
     * @param datetimeStr String en formato "YYYY-MM-DD HH:MM:SS"
     * @param dt DateTime donde almacenar el resultado
     * @return true si el parsing fue exitoso
     */
    bool parseDateTime(const String& datetimeStr, RTC_DS1302::DateTime& dt) {
        // Formato esperado: "YYYY-MM-DD HH:MM:SS"
        if (datetimeStr.length() != 19) return false;
        
        // Verificar caracteres separadores
        if (datetimeStr.charAt(4) != '-' || datetimeStr.charAt(7) != '-' || 
            datetimeStr.charAt(10) != ' ' || datetimeStr.charAt(13) != ':' || 
            datetimeStr.charAt(16) != ':') {
            return false;
        }
        
        int year = datetimeStr.substring(0, 4).toInt();
        int month = datetimeStr.substring(5, 7).toInt();
        int day = datetimeStr.substring(8, 10).toInt();
        int hour = datetimeStr.substring(11, 13).toInt();
        int minute = datetimeStr.substring(14, 16).toInt();
        int second = datetimeStr.substring(17, 19).toInt();
        
        dt.year = (year >= 2000) ? year - 2000 : year;
        dt.month = month;
        dt.day = day;
        dt.hour = hour;
        dt.minute = minute;
        dt.second = second;
        dt.dayOfWeek = calculateDayOfWeek(year, month, day);
        
        return dt.isValid();
    }
    
    /**
     * @brief Calcular día de la semana
     * @param year Año completo (ej. 2025)
     * @param month Mes (1-12)
     * @param day Día (1-31)
     * @return Día de la semana (1=Domingo, 7=Sábado)
     */
    uint8_t calculateDayOfWeek(int year, int month, int day) {
        // Algoritmo de Zeller
        if (month < 3) {
            month += 12;
            year--;
        }
        
        int q = day;
        int m = month;
        int k = year % 100;
        int j = year / 100;
        
        int h = (q + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
        return ((h + 5) % 7) + 1;
    }
    
    /**
     * @brief Convertir día de la semana a string
     * @param dayOfWeek Día de la semana (1-7)
     * @return Nombre del día
     */
    String dayOfWeekToString(uint8_t dayOfWeek) {
        switch (dayOfWeek) {
            case 1: return "Domingo";
            case 2: return "Lunes";
            case 3: return "Martes";
            case 4: return "Miércoles";
            case 5: return "Jueves";
            case 6: return "Viernes";
            case 7: return "Sábado";
            default: return "Desconocido";
        }
    }
    
    /**
     * @brief Convertir código de error RTC a string
     * @param error Código de error
     * @return Descripción del error
     */
    String rtcErrorToString(RTC_DS1302::RTCError error) {
        switch (error) {
            case RTC_DS1302::RTCError::NO_ERROR:
                return "Sin error";
            case RTC_DS1302::RTCError::COMMUNICATION_FAILED:
                return "Fallo de comunicación";
            case RTC_DS1302::RTCError::CLOCK_STOPPED:
                return "Reloj detenido";
            case RTC_DS1302::RTCError::WRITE_VERIFICATION_FAILED:
                return "Verificación de escritura falló";
            case RTC_DS1302::RTCError::INITIALIZATION_FAILED:
                return "Inicialización falló";
            default:
                return "Error desconocido";
        }
    }
};

// Instancia global para uso en otros módulos
RTCSerialCommands* globalRTCSerialCommands = nullptr;

/**
 * @brief Inicializar comandos seriales del RTC
 * @param rtcInstance Instancia del RTC
 */
void initializeRTCSerialCommands(RTC_DS1302* rtcInstance) {
    if (globalRTCSerialCommands) {
        delete globalRTCSerialCommands;
    }
    
    globalRTCSerialCommands = new RTCSerialCommands(rtcInstance);
    LOG_INFO("[RTC_SERIAL] Comandos seriales del RTC inicializados");
    
    // Mostrar ayuda inicial
    Serial.println("\n🕐 Comandos RTC disponibles. Usa 'rtc_help' para más información.");
}

/**
 * @brief Procesar comandos seriales RTC
 * Debe llamarse en el loop principal
 */
void processRTCSerialCommands() {
    if (globalRTCSerialCommands) {
        globalRTCSerialCommands->processCommands();
    }
}
