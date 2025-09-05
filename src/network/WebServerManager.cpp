/**
 * @file WebServerManager.cpp
 * @brief Implementación del gestor del servidor HTTP para el sistema de riego.
 * 
 * **CONCEPTO EDUCATIVO - SEPARACIÓN DE RESPONSABILIDADES**:
 * Esta implementación demuestra cómo aislar la lógica del servidor HTTP
 * en una clase especializada, mejorando la mantenibilidad y testabilidad.
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Arquitectura Modular
 * @date 2025
 */

#include <cstdint>
#include "network/WebServerManager.h"
#include "core/SystemManager.h"
#include "core/EventBus.h"
#include "utils/Logger.h"
#include "drivers/ServoPWMController.h"
#include "core/ConfigManager.h"
#include "core/SystemConfig.h"
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

/**
 * @brief Autenticar una solicitud HTTP
 * @param request Petición HTTP a autenticar
 * @return true si la autenticación es exitosa, false en caso contrario
 */
bool WebServerManager::authenticateRequest(AsyncWebServerRequest *request) {
    // Si la autenticación web está deshabilitada, permitir acceso
    if (!SecurityConfig::ENABLE_WEB_AUTHENTICATION) {
        return true;
    }

    // Permitir acceso público al endpoint de estado
    if (request->url() == "/api/v1/status") {
        return true;
    }

    // Requerir autenticación para todos los endpoints de configuración
    if (request->url().startsWith("/api/config")) {
        if (!request->authenticate(SecurityConfig::DEFAULT_WEB_USERNAME, SecurityConfig::DEFAULT_WEB_PASSWORD)) {
            LOG_WARNING("[WEBSERVER] Intento de acceso no autenticado a configuración desde: " + 
                       request->client()->remoteIP().toString());
            request->requestAuthentication();
            return false;
        }
        return true;
    }

    // Comportamiento por defecto para otros endpoints
    if (request->authenticate(SecurityConfig::DEFAULT_WEB_USERNAME, SecurityConfig::DEFAULT_WEB_PASSWORD)) {
        return true;
    }
    
    LOG_WARNING("[WEBSERVER] Intento de acceso no autenticado desde: " + request->client()->remoteIP().toString());
    request->requestAuthentication();
    return false;
}

/**
 * @brief Constructor del WebServerManager
 * @param port Puerto del servidor HTTP
 */
WebServerManager::WebServerManager(uint16_t port) 
    : server(new AsyncWebServer(port)), systemManager(nullptr), isServerRunning(false) {
    LOG_INFO("[WEBSERVER] Gestor del servidor web creado en puerto " + String(port));
}

/**
 * @brief Destructor del WebServerManager
 */
WebServerManager::~WebServerManager() {
    stop();
    delete server;
    LOG_INFO("[WEBSERVER] Gestor del servidor web destruido");
}

/**
 * @brief Inicializar el servidor web
 * @param systemManager Referencia al gestor del sistema
 */
void WebServerManager::initialize(SystemManager* sysManager) {
    systemManager = sysManager;
    LOG_INFO("[WEBSERVER] Inicializando servidor web...");
    
    setupStaticFiles();
    setupRESTEndpoints();
    setupErrorHandling();
    
    LOG_INFO("[WEBSERVER] Servidor web inicializado correctamente");
}

/**
 * @brief Configurar archivos estáticos (SPIFFS)
 */
void WebServerManager::setupStaticFiles() {
    LOG_INFO("[WEBSERVER] Configurando archivos estáticos...");
    
    if (!SPIFFS.begin(true)) {
        LOG_ERROR("[WEBSERVER] Fallo al inicializar SPIFFS");
        return;
    }
    
    // Servir archivos estáticos desde SPIFFS
    server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    
    // Servir recursos específicos
    server->on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/LOGO.png", "image/png");
    });
    
    LOG_INFO("[WEBSERVER] Archivos estáticos configurados");
}

/**
 * @brief Configurar endpoints REST
 */
void WebServerManager::setupRESTEndpoints() {
    LOG_INFO("[WEBSERVER] Configurando endpoints REST...");
    
    // Endpoint de estado del sistema (público)
    server->on("/api/v1/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        LOG_DEBUG("[WEBSERVER] Petición de estado público desde: " + request->client()->remoteIP().toString());
        
        if (!systemManager) {
            request->send(503, "application/json", "{\"error\":\"Sistema no inicializado\"}");
            return;
        }
        
        // Obtener información del sistema
        ServoPWMController* ctrl = systemManager->getIrrigationController();
        String state = ctrl ? ServoPWMController::stateToString(ctrl->getCurrentState()) : "UNKNOWN";
        int activeZone = ctrl ? ctrl->getCurrentActiveZone() : 0;
        int remaining = ctrl ? ctrl->getRemainingIrrigationTime() : 0;
        
        // Crear respuesta JSON
        String json = "{\"state\":\"" + state + "\",";
        json += "\"activeZone\":" + String(activeZone) + ",";
        json += "\"remainingTime\":" + String(remaining) + ",";
        json += "\"memoryFree\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"status\":\"operational\"}";
        
        request->send(200, "application/json", json);
    });
    
    // Endpoint para configurar RTC
    server->on("/api/config/rtc", HTTP_POST, [this](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Configuración RTC desde web");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        if (!systemManager) {
            request->send(503, "application/json", "{\"error\":\"Sistema no inicializado\"}");
            return;
        }
        
        // Extraer parámetros
        uint8_t year = request->getParam("year") ? request->getParam("year")->value().toInt() : 25;
        uint8_t month = request->getParam("month") ? request->getParam("month")->value().toInt() : 1;
        uint8_t day = request->getParam("day") ? request->getParam("day")->value().toInt() : 1;
        uint8_t dayOfWeek = request->getParam("dayOfWeek") ? request->getParam("dayOfWeek")->value().toInt() : 1;
        uint8_t hour = request->getParam("hour") ? request->getParam("hour")->value().toInt() : 0;
        uint8_t minute = request->getParam("minute") ? request->getParam("minute")->value().toInt() : 0;
        uint8_t second = request->getParam("second") ? request->getParam("second")->value().toInt() : 0;
        
        // Configurar RTC
        if (systemManager->setRTCDateTime(year, month, day, dayOfWeek, hour, minute, second)) {
            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"RTC configurado\"}");
            
            // Publicar evento de configuración
            EventBus::getInstance().publishSimple(EventType::RTC_CONFIGURED);
        } else {
            request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Error al configurar RTC\"}");
        }
    });

    // **FASE 5: ENDPOINTS DE CONFIGURACIÓN DINÁMICA**
    setupConfigurationEndpoints();
    
    LOG_INFO("[WEBSERVER] Endpoints REST configurados");
}

/**
 * @brief Configurar endpoints de configuración dinámica
 */
void WebServerManager::setupConfigurationEndpoints() {
    LOG_INFO("[WEBSERVER] Configurando endpoints de configuración dinámica...");
    
    ConfigManager& configManager = ConfigManager::getInstance();
    
    // GET /api/config - Obtener configuración completa
    server->on("/api/config", HTTP_GET, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición GET de configuración completa");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        String jsonConfig = configManager.exportConfig();
        request->send(200, "application/json", jsonConfig);
    });
    
    // PUT /api/config - Actualizar configuración completa
    server->on("/api/config", HTTP_PUT, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición PUT de configuración completa");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        if (request->contentType() != "application/json") {
            request->send(400, "application/json", "{\"error\":\"Content-Type debe ser application/json\"}");
            return;
        }
        
        if (request->contentLength() == 0) {
            request->send(400, "application/json", "{\"error\":\"Cuerpo de solicitud vacío\"}");
            return;
        }
        
        String jsonBody = request->getParam("plain", true)->value();
        
        if (configManager.importConfig(jsonBody)) {
            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuración actualizada\"}");
            
            // Publicar evento de configuración actualizada
            EventBus::getInstance().publishSimple(EventType::CONFIG_UPDATED);
        } else {
            request->send(400, "application/json", "{\"error\":\"Configuración inválida o error al guardar\"}");
        }
    });
    
    // GET /api/config/defaults - Obtener configuración por defecto
    server->on("/api/config/defaults", HTTP_GET, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición GET de configuración por defecto");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        // Crear configuración por defecto sin alterar la configuración actual
        SystemConfig defaultConfig;
        ConfigManager::getInstance().resetToDefaults(); // Esto carga los valores por defecto
        String jsonConfig = configManager.exportConfig();
        request->send(200, "application/json", jsonConfig);
    });
    
    // POST /api/config/reset - Restablecer configuración por defecto
    server->on("/api/config/reset", HTTP_POST, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición POST para resetear configuración");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        if (configManager.resetToDefaults()) {
            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuración restablecida a valores por defecto\"}");
            
            // Publicar evento de configuración resetada
            EventBus::getInstance().publishSimple(EventType::CONFIG_RESET);
        } else {
            request->send(500, "application/json", "{\"error\":\"Error al restablecer configuración\"}");
        }
    });
    
    // GET /api/config/backup - Listar backups disponibles
    server->on("/api/config/backup", HTTP_GET, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición GET de lista de backups");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        // Por ahora, solo tenemos un backup, pero podemos expandir esto
        DynamicJsonDocument doc(256);
        doc["backups"] = 1;
        doc["last_backup"] = "config_backup.json";
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // POST /api/config/backup - Crear nuevo backup
    server->on("/api/config/backup", HTTP_POST, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición POST para crear backup");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        if (configManager.createBackup()) {
            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Backup creado exitosamente\"}");
        } else {
            request->send(500, "application/json", "{\"error\":\"Error al crear backup\"}");
        }
    });
    
    // POST /api/config/backup/restore - Restaurar desde backup
    server->on("/api/config/backup/restore", HTTP_POST, [this, &configManager](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Petición POST para restaurar backup");
        
        // Verificar autenticación
        if (!this->authenticateRequest(request)) {
            return;
        }
        
        if (configManager.restoreBackup()) {
            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Backup restaurado exitosamente\"}");
            
            // Publicar evento de configuración restaurada
            EventBus::getInstance().publishSimple(EventType::CONFIG_RESTORED);
        } else {
            request->send(500, "application/json", "{\"error\":\"Error al restaurar backup\"}");
        }
    });
    
    LOG_INFO("[WEBSERVER] Endpoints de configuración dinámica configurados");
}

/**
 * @brief Configurar manejo de errores
 */
void WebServerManager::setupErrorHandling() {
    LOG_INFO("[WEBSERVER] Configurando manejo de errores...");
    
    // Manejo de recursos no encontrados
    server->onNotFound([](AsyncWebServerRequest *request){
        LOG_INFO("[WEBSERVER] Recurso no encontrado: " + request->url());
        
        String message = "<!DOCTYPE html><html><head><title>404 - No Encontrado</title></head><body>";
        message += "<h1>Recurso No Encontrado</h1>";
        message += "<p>El recurso <strong>" + request->url() + "</strong> no existe.</p>";
        message += "<p><a href='/'>Volver al Panel de Control</a></p>";
        message += "<hr><small>Sistema de Riego Inteligente v3.2</small></body></html>";
        
        request->send(404, "text/html", message);
    });
    
    LOG_INFO("[WEBSERVER] Manejo de errores configurado");
}

/**
 * @brief Iniciar el servidor web
 */
void WebServerManager::start() {
    if (isServerRunning) {
        LOG_INFO("[WEBSERVER] Servidor ya está ejecutándose");
        return;
    }
    
    LOG_INFO("[WEBSERVER] Iniciando servidor web...");
    server->begin();
    isServerRunning = true;
    
    // Configurar mDNS para acceso por nombre
    if (MDNS.begin("sistemariego")) {
        MDNS.addService("http", "tcp", 80);
        LOG_INFO("[WEBSERVER] mDNS configurado: http://sistemariego.local");
    } else {
        LOG_ERROR("[WEBSERVER] Fallo al configurar mDNS");
    }
    
    LOG_INFO("[WEBSERVER] Servidor web iniciado correctamente");
}

/**
 * @brief Detener el servidor web
 */
void WebServerManager::stop() {
    if (!isServerRunning) {
        return;
    }
    
    LOG_INFO("[WEBSERVER] Deteniendo servidor web...");
    server->end();
    isServerRunning = false;
    LOG_INFO("[WEBSERVER] Servidor web detenido");
}

/**
 * @brief Obtener el servidor HTTP
 * @return Referencia al servidor AsyncWebServer
 */
AsyncWebServer& WebServerManager::getServer() {
    return *server;
}
