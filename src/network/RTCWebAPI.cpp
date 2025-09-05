/**
 * @file RTCWebAPI.cpp
 * @brief API Web para configuración y gestión del RTC DS1302
 * 
 * Este archivo implementa endpoints específicos para la configuración
 * del RTC (Real Time Clock) a través de la interfaz web.
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0
 * @date 2025
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "../include/ProjectConfig.h"
#include "drivers/RTC_DS1302.h"
#include "utils/Logger.h"

class RTCWebAPI {
private:
    RTC_DS1302* rtcInstance;
    
public:
    RTCWebAPI(RTC_DS1302* rtc) : rtcInstance(rtc) {}
    
    /**
     * @brief Configurar endpoints RTC en el servidor web
     * @param server Servidor web donde configurar los endpoints
     */
    void setupEndpoints(AsyncWebServer* server) {
        if (!server || !rtcInstance) {
            LOG_ERROR("[RTC_API] Servidor o instancia RTC no válidos");
            return;
        }
        
        // GET /api/rtc - Obtener fecha y hora actual
        server->on("/api/rtc", HTTP_GET, [this](AsyncWebServerRequest *request){
            this->handleGetDateTime(request);
        });
        
        // POST /api/rtc - Establecer fecha y hora
        server->on("/api/rtc", HTTP_POST, [this](AsyncWebServerRequest *request){}, 
                   NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            this->handleSetDateTime(request, data, len, index, total);
        });
        
        // GET /api/rtc/status - Obtener estado del RTC
        server->on("/api/rtc/status", HTTP_GET, [this](AsyncWebServerRequest *request){
            this->handleGetStatus(request);
        });
        
        LOG_INFO("[RTC_API] Endpoints RTC configurados");
    }
    
private:
    /**
     * @brief Manejar petición GET para obtener fecha/hora actual
     * @param request Petición HTTP
     */
    void handleGetDateTime(AsyncWebServerRequest *request) {
        LOG_INFO("[RTC_API] Petición GET fecha/hora actual");
        
        if (!rtcInstance->isInitialized()) {
            request->send(503, "application/json", 
                         "{\"error\":\"RTC no inicializado\",\"code\":\"RTC_NOT_INITIALIZED\"}");
            return;
        }
        
        RTC_DS1302::DateTime dt = rtcInstance->getDateTime();
        if (!dt.isValid()) {
            request->send(500, "application/json", 
                         "{\"error\":\"Error al leer RTC\",\"code\":\"RTC_READ_ERROR\"}");
            return;
        }
        
        // Crear respuesta JSON
        DynamicJsonDocument doc(256);
        doc["success"] = true;
        doc["datetime"] = formatDateTime(dt);
        doc["timestamp"] = dateTimeToTimestamp(dt);
        doc["year"] = 2000 + dt.year;
        doc["month"] = dt.month;
        doc["day"] = dt.day;
        doc["dayOfWeek"] = dt.dayOfWeek;
        doc["hour"] = dt.hour;
        doc["minute"] = dt.minute;
        doc["second"] = dt.second;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    }
    
    /**
     * @brief Manejar petición POST para establecer fecha/hora
     * @param request Petición HTTP
     * @param data Datos del cuerpo de la petición
     * @param len Longitud de los datos
     * @param index Índice actual (para datos grandes)
     * @param total Tamaño total de los datos
     */
    void handleSetDateTime(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        LOG_INFO("[RTC_API] Petición POST establecer fecha/hora");
        
        if (!rtcInstance->isInitialized()) {
            request->send(503, "application/json", 
                         "{\"error\":\"RTC no inicializado\",\"code\":\"RTC_NOT_INITIALIZED\"}");
            return;
        }
        
        // Construir JSON completo si los datos vienen en fragmentos
        static String jsonBuffer;
        if (index == 0) {
            jsonBuffer = "";
        }
        
        for (size_t i = 0; i < len; i++) {
            jsonBuffer += (char)data[i];
        }
        
        // Procesar solo cuando tengamos todos los datos
        if (index + len != total) {
            return;
        }
        
        // Parsear JSON
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, jsonBuffer);
        if (error) {
            request->send(400, "application/json", 
                         "{\"error\":\"JSON inválido\",\"code\":\"INVALID_JSON\"}");
            return;
        }
        
        // Extraer fecha y hora del JSON
        RTC_DS1302::DateTime dt;
        
        // Permitir múltiples formatos de entrada
        if (doc.containsKey("datetime")) {
            // Formato: "YYYY-MM-DD HH:MM:SS"
            String datetimeStr = doc["datetime"].as<String>();
            if (!parseDateTime(datetimeStr, dt)) {
                request->send(400, "application/json", 
                             "{\"error\":\"Formato de fecha/hora inválido\",\"code\":\"INVALID_DATETIME_FORMAT\"}");
                return;
            }
        } else if (doc.containsKey("year") && doc.containsKey("month") && doc.containsKey("day")) {
            // Formato: campos individuales
            int year = doc["year"];
            dt.year = (year >= 2000) ? year - 2000 : year;
            dt.month = doc["month"];
            dt.day = doc["day"];
            dt.dayOfWeek = doc.containsKey("dayOfWeek") ? doc["dayOfWeek"] : calculateDayOfWeek(dt.year + 2000, dt.month, dt.day);
            dt.hour = doc.containsKey("hour") ? doc["hour"] : 0;
            dt.minute = doc.containsKey("minute") ? doc["minute"] : 0;
            dt.second = doc.containsKey("second") ? doc["second"] : 0;
        } else {
            request->send(400, "application/json", 
                         "{\"error\":\"Campos requeridos faltantes\",\"code\":\"MISSING_REQUIRED_FIELDS\"}");
            return;
        }
        
        // Validar fecha/hora
        if (!dt.isValid()) {
            request->send(400, "application/json", 
                         "{\"error\":\"Fecha/hora inválida\",\"code\":\"INVALID_DATETIME\"}");
            return;
        }
        
        // Establecer fecha/hora en RTC
        if (rtcInstance->setDateTime(dt)) {
            DynamicJsonDocument responseDoc(256);
            responseDoc["success"] = true;
            responseDoc["message"] = "Fecha/hora establecida correctamente";
            responseDoc["datetime"] = formatDateTime(dt);
            
            String response;
            serializeJson(responseDoc, response);
            request->send(200, "application/json", response);
            
            LOG_INFO("[RTC_API] Fecha/hora establecida: " + formatDateTime(dt));
        } else {
            request->send(500, "application/json", 
                         "{\"error\":\"Error al establecer fecha/hora en RTC\",\"code\":\"RTC_WRITE_ERROR\"}");
        }
    }
    
    /**
     * @brief Manejar petición GET para obtener estado del RTC
     * @param request Petición HTTP
     */
    void handleGetStatus(AsyncWebServerRequest *request) {
        LOG_INFO("[RTC_API] Petición GET estado RTC");
        
        DynamicJsonDocument doc(512);
        doc["initialized"] = rtcInstance->isInitialized();
        doc["running"] = rtcInstance->isRunning();
        doc["lastError"] = rtcErrorToString(rtcInstance->getLastError());
        
        // Información adicional si está inicializado
        if (rtcInstance->isInitialized()) {
            RTC_DS1302::DateTime dt = rtcInstance->getDateTime();
            if (dt.isValid()) {
                doc["currentDateTime"] = formatDateTime(dt);
                doc["timestamp"] = dateTimeToTimestamp(dt);
            } else {
                doc["currentDateTime"] = nullptr;
                doc["readError"] = true;
            }
        }
        
        // Información de configuración
        doc["pins"]["rst"] = globalConfig.rtc_rst;
        doc["pins"]["sclk"] = globalConfig.rtc_sclk;
        doc["pins"]["io"] = globalConfig.rtc_io;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    }
    
    /**
     * @brief Formatear DateTime a string legible
     * @param dt DateTime a formatear
     * @return String formateado (YYYY-MM-DD HH:MM:SS)
     */
    String formatDateTime(const RTC_DS1302::DateTime& dt) {
        char buffer[32];
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
                2000 + dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        return String(buffer);
    }
    
    /**
     * @brief Convertir DateTime a timestamp Unix
     * @param dt DateTime a convertir
     * @return Timestamp Unix
     */
    unsigned long dateTimeToTimestamp(const RTC_DS1302::DateTime& dt) {
        // Implementación simple de conversión a timestamp
        // Para una implementación más robusta, considerar usar librerías de tiempo
        return 0; // Placeholder - implementar si se necesita
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
     * @param year Año (formato completo, ej. 2025)
     * @param month Mes (1-12)
     * @param day Día (1-31)
     * @return Día de la semana (1=Domingo, 7=Sábado)
     */
    uint8_t calculateDayOfWeek(int year, int month, int day) {
        // Algoritmo de Zeller para calcular día de la semana
        if (month < 3) {
            month += 12;
            year--;
        }
        
        int q = day;
        int m = month;
        int k = year % 100;
        int j = year / 100;
        
        int h = (q + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
        
        // Convertir resultado de Zeller (0=Sábado) a formato RTC (1=Domingo)
        return ((h + 5) % 7) + 1;
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
RTCWebAPI* globalRTCWebAPI = nullptr;

/**
 * @brief Inicializar API Web del RTC
 * @param server Servidor web
 * @param rtcInstance Instancia del RTC
 */
void initializeRTCWebAPI(AsyncWebServer* server, RTC_DS1302* rtcInstance) {
    if (globalRTCWebAPI) {
        delete globalRTCWebAPI;
    }
    
    globalRTCWebAPI = new RTCWebAPI(rtcInstance);
    globalRTCWebAPI->setupEndpoints(server);
    LOG_INFO("[RTC_API] API Web del RTC inicializada");
}
