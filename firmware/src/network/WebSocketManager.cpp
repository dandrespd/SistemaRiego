/**
 * @file WebSocketManager.cpp
 * @brief Implementación del gestor de WebSockets para comunicación en tiempo real.
 * 
 * Esta implementación demuestra cómo crear un sistema de comunicación
 * bidireccional eficiente entre un ESP32 y múltiples clientes web.
 */

#include <Arduino.h>
#include "Utils.h"
#include "WebSocketManager.h"
#include "SET_PIN.h"
#include "IN_DIGITAL.h"
#include <ArduinoJson.h>

// =============================================================================
// Constructor y Destructor
// =============================================================================

WebSocketManager::WebSocketManager(const char* path, ServoPWMController* controller)
    : webSocket(nullptr)
    , irrigationController(controller)
    , lastStatusUpdate(0)
    , lastHeartbeat(0)
    , statusChanged(false)
    , totalConnectionsCount(0)
    , messagesSentCount(0)
    , messagesReceivedCount(0)
{
    // Crear instancia del servidor WebSocket
    webSocket = new AsyncWebSocket(path);
    
    // Inicializar estructura de estado
    memset(&lastKnownStatus, 0, sizeof(SystemStatus));
}

WebSocketManager::~WebSocketManager() {
    if (webSocket) {
        webSocket->closeAll();
        delete webSocket;
    }
}

// =============================================================================
// Inicialización y configuración
// =============================================================================

bool WebSocketManager::initialize() {
    if (!webSocket) {
        DEBUG_PRINTLN("❌ [WebSocket] Error: WebSocket no inicializado");
        return false;
    }
    
    // **CONFIGURACIÓN DE CALLBACK**
    // Usamos lambda para capturar 'this' y poder llamar métodos de la clase
    webSocket->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                             AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->handleWebSocketEvent(server, client, type, arg, data, len);
    });
    
    DEBUG_PRINTLN("✅ [WebSocket] Servidor WebSocket inicializado correctamente");
    return true;
}

// =============================================================================
// Procesamiento principal
// =============================================================================

void WebSocketManager::update() {
    if (!webSocket) return;
    
    unsigned long currentTime = millis();
    
    // **ACTUALIZACIÓN PERIÓDICA DE ESTADO**
    if (currentTime - lastStatusUpdate >= WebSocketConfig::STATUS_UPDATE_INTERVAL_MS) {
        if (hasStatusChanged()) {
            broadcastStatusUpdate();
        }
        lastStatusUpdate = currentTime;
    }
    
    // **HEARTBEAT PERIÓDICO**
    if (currentTime - lastHeartbeat >= WebSocketConfig::HEARTBEAT_INTERVAL_MS) {
        sendHeartbeat();
        lastHeartbeat = currentTime;
    }
    
    // **LIMPIEZA DE CONEXIONES INACTIVAS**
    static unsigned long lastCleanup = 0;
    if (currentTime - lastCleanup >= 60000) { // Cada minuto
        cleanupInactiveConnections();
        lastCleanup = currentTime;
    }
    
    // **PROCESAMIENTO DE WEBSOCKET**
    webSocket->cleanupClients();
}

// =============================================================================
// Comunicación con clientes
// =============================================================================

void WebSocketManager::broadcastStatusUpdate() {
    if (!webSocket || webSocket->count() == 0) return;
    
    String statusJson = serializeSystemStatus();
    
    if (statusJson.length() > 0) {
        webSocket->textAll(statusJson);
        messagesSentCount++;
        
        VERBOSE_PRINTLN("[WebSocket] Estado enviado a " + String(webSocket->count()) + " clientes");
        
        // Actualizar último estado conocido
        lastKnownStatus = getCurrentSystemStatus();
        statusChanged = false;
    }
}

void WebSocketManager::broadcastError(const String& errorMessage, const String& severity) {
    if (!webSocket || webSocket->count() == 0) return;
    
    // **CREAR MENSAJE DE ERROR ESTRUCTURADO**
    StaticJsonDocument<384> doc;
    doc["type"] = "error";
    doc["severity"] = severity;
    doc["message"] = errorMessage;
    doc["timestamp"] = millis();
    
    String message;
    serializeJson(doc, message);
    
    webSocket->textAll(message);
    messagesSentCount++;
    
    DEBUG_PRINTLN("🚨 [WebSocket] Error enviado: " + errorMessage);
}

void WebSocketManager::forceStatusUpdate() {
    statusChanged = true;
}

// =============================================================================
// Manejo de eventos WebSocket
// =============================================================================

void WebSocketManager::handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                                           AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch(type) {
        case WS_EVT_CONNECT:
            {
                totalConnectionsCount++;
                logWebSocketEvent("Cliente conectado", client->id());
                
                // **ENVIAR ESTADO ACTUAL AL NUEVO CLIENTE**
                String statusJson = serializeSystemStatus();
                client->text(statusJson);
                messagesSentCount++;
                
                DEBUG_PRINTLN("🔗 [WebSocket] Cliente " + String(client->id()) + " conectado desde " + 
                             client->remoteIP().toString());
            }
            break;
            
        case WS_EVT_DISCONNECT:
            logWebSocketEvent("Cliente desconectado", client->id());
            DEBUG_PRINTLN("🔌 [WebSocket] Cliente " + String(client->id()) + " desconectado");
            break;
            
        case WS_EVT_DATA:
            {
                AwsFrameInfo* info = (AwsFrameInfo*)arg;
                if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                    data[len] = 0; // Null terminator
                    String message = (char*)data;
                    handleClientMessage(client, message);
                    messagesReceivedCount++;
                }
            }
            break;
            
        case WS_EVT_PONG:
            // Cliente respondió al ping
            VERBOSE_PRINTLN("[WebSocket] Pong recibido de cliente " + String(client->id()));
            break;
            
        case WS_EVT_ERROR:
            DEBUG_PRINTLN("❌ [WebSocket] Error en cliente " + String(client->id()));
            break;
    }
}

void WebSocketManager::handleClientMessage(AsyncWebSocketClient* client, const String& message) {
    DEBUG_PRINTLN("📥 [WebSocket] Mensaje de cliente " + String(client->id()) + ": " + message);
    
    // **PARSEAR MENSAJE JSON**
    StaticJsonDocument<WebSocketConfig::MAX_MESSAGE_SIZE> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        DEBUG_PRINTLN("❌ [WebSocket] Error parsing JSON: " + String(error.c_str()));
        
        // Enviar mensaje de error al cliente
        DynamicJsonDocument errorDoc(256);
        errorDoc["type"] = "error";
        errorDoc["message"] = "JSON inválido";
        String errorMsg;
        serializeJson(errorDoc, errorMsg);
        client->text(errorMsg);
        return;
    }
    
    // **PROCESAR COMANDO**
    String command = doc["command"] | "";
    String parameters = doc["parameters"] | "";
    
    if (command.length() > 0) {
        bool success = processClientCommand(client->id(), command, parameters);
        
        // **ENVIAR RESPUESTA AL CLIENTE**
        DynamicJsonDocument responseDoc(256);
        responseDoc["type"] = "response";
        responseDoc["command"] = command;
        responseDoc["success"] = success;
        responseDoc["timestamp"] = millis();
        
        String response;
        serializeJson(responseDoc, response);
        client->text(response);
    }
}

// =============================================================================
// Procesamiento de comandos
// =============================================================================

// Helper: parse query string like "key1=val1&key2=val2" into provided keys
static void parseQueryParams(const String& params, String& zoneStr, String& timeStr, String& durationStr, String& enabledStr) {
    zoneStr = ""; timeStr = ""; durationStr = ""; enabledStr = "";
    int start = 0;
    while (start < (int)params.length()) {
        int eq = params.indexOf('=', start);
        if (eq < 0) break;
        int amp = params.indexOf('&', eq + 1);
        String key = params.substring(start, eq);
        String val = amp >= 0 ? params.substring(eq + 1, amp) : params.substring(eq + 1);
        if (key == "zone") zoneStr = val;
        else if (key == "time") timeStr = val;
        else if (key == "duration") durationStr = val;
        else if (key == "enabled") enabledStr = val;
        if (amp < 0) break; else start = amp + 1;
    }
}

bool WebSocketManager::processClientCommand(uint32_t clientId, const String& command, const String& parameters) {
    DEBUG_PRINTLN("🎯 [WebSocket] Ejecutando comando: " + command);
    
    if (!validateCommand(command, parameters)) {
        DEBUG_PRINTLN("❌ [WebSocket] Comando inválido: " + command);
        return false;
    }
    
    if (!irrigationController) {
        DEBUG_PRINTLN("❌ [WebSocket] Controlador de riego no disponible");
        return false;
    }
    
    // **PROCESAR COMANDOS ESPECÍFICOS**
    if (command == "start_irrigation") {
        return irrigationController->startIrrigationCycle(true);
    }
    else if (command == "stop_irrigation") {
        irrigationController->stopIrrigationCycle();
        return true;
    }
    else if (command == "emergency_stop") {
        irrigationController->emergencyStopAll();
        broadcastError("Parada de emergencia activada por cliente " + String(clientId), "WARNING");
        return true;
    }
    else if (command == "open_zone") {
        String zoneStr, timeStr, durationStr, enabledStr; parseQueryParams(parameters, zoneStr, timeStr, durationStr, enabledStr);
        uint8_t zone = (uint8_t) zoneStr.toInt();
        uint32_t duration = durationStr.length() ? (uint32_t) durationStr.toInt() : 300;
        
        if (zone > 0 && zone <= NUM_SERVOS) {
            return irrigationController->openZoneValve(zone, duration);
        }
    }
    else if (command == "close_zone") {
        String zoneStr, timeStr, durationStr, enabledStr; parseQueryParams(parameters, zoneStr, timeStr, durationStr, enabledStr);
        uint8_t zone = (uint8_t) zoneStr.toInt();
        
        if (zone > 0 && zone <= NUM_SERVOS) {
            return irrigationController->closeZoneValve(zone);
        }
    }
    else if (command == "set_zone_time") {
        String zoneStr, timeStr, durationStr, enabledStr; parseQueryParams(parameters, zoneStr, timeStr, durationStr, enabledStr);
        uint8_t zone = (uint8_t) zoneStr.toInt();
        uint32_t time = timeStr.length() ? (uint32_t) timeStr.toInt() : 300;
        
        if (zone > 0 && zone <= NUM_SERVOS) {
            return irrigationController->setZoneIrrigationTime(zone, time);
        }
    }
    else if (command == "enable_zone") {
        String zoneStr, timeStr, durationStr, enabledStr; parseQueryParams(parameters, zoneStr, timeStr, durationStr, enabledStr);
        uint8_t zone = (uint8_t) zoneStr.toInt();
        bool enabled = enabledStr.length() ? (enabledStr == "true" || enabledStr == "1") : true;
        
        if (zone > 0 && zone <= NUM_SERVOS) {
            return irrigationController->setZoneEnabled(zone, enabled);
        }
    }
    else if (command == "get_status") {
        // Forzar actualización de estado
        forceStatusUpdate();
        return true;
    }
    
    DEBUG_PRINTLN("❌ [WebSocket] Comando no reconocido: " + command);
    return false;
}

// =============================================================================
// Serialización y estado
// =============================================================================

String WebSocketManager::serializeSystemStatus() {
    SystemStatus status = getCurrentSystemStatus();

    StaticJsonDocument<1024> doc;

    // Tipo y tiempos
    doc["type"] = "status_update";
    doc["timestamp"] = status.timestamp;

    // Sistema
    JsonObject system = doc["system"].to<JsonObject>();
    system["uptime"] = status.systemUptime;
    system["freeMemory"] = status.freeMemory;
    system["hasErrors"] = status.hasErrors;

    // Riego
    JsonObject irrigation = doc["irrigation"].to<JsonObject>();
    irrigation["state"] = status.irrigationState;
    irrigation["activeZone"] = status.activeZone;
    irrigation["remainingTime"] = status.remainingTime;
    irrigation["totalCycles"] = status.totalCycles;

    // Sensores
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    sensors["humidity"] = status.humidityPercent;
    sensors["humidityThreshold"] = status.humidityThreshold;

    // Zonas
    JsonArray zones = doc["zones"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        JsonObject zone = zones.createNestedObject();
        zone["number"] = i + 1;
        zone["enabled"] = status.zonesEnabled[i];
        zone["irrigationTime"] = status.zoneTimes[i];
    }

    String result;
    serializeJson(doc, result);
    return result;
}

SystemStatus WebSocketManager::getCurrentSystemStatus() {
    SystemStatus status;
    memset(&status, 0, sizeof(SystemStatus));
    
    status.timestamp = millis();
    status.systemUptime = millis() / 1000;
    status.freeMemory = ESP.getFreeHeap();
    status.hasErrors = false;
    
    if (irrigationController) {
        // Estado del sistema de riego
        IrrigationState currentState = irrigationController->getCurrentState();
        status.irrigationState = String(ServoPWMController::stateToString(currentState));
        status.activeZone = irrigationController->getCurrentActiveZone();
        status.remainingTime = irrigationController->getRemainingIrrigationTime();
        status.hasErrors = irrigationController->hasErrors();
        
        // Estadísticas
        uint32_t cycles, totalTime, uptime;
        irrigationController->getSystemStatistics(cycles, totalTime, uptime);
        status.totalCycles = cycles;
        
        // Estado de zonas
        for (uint8_t i = 0; i < NUM_SERVOS; i++) {
            const ZoneInfo* zoneInfo = irrigationController->getZoneInfo(i + 1);
            if (zoneInfo) {
                status.zonesEnabled[i] = zoneInfo->isEnabled;
                status.zoneTimes[i] = zoneInfo->config.irrigationTime;
            }
        }
    }
    
    // **SENSOR DE HUMEDAD DIGITAL -> porcentaje aproximado**
    InDigital humiditySensor(IN_DIGITAL);
    bool isDry = humiditySensor.isHigh();
    status.humidityPercent = isDry ? 10 : 80; // aproximación (10% seco, 80% húmedo)
    status.humidityThreshold = 50;
    
    return status;
}

bool WebSocketManager::hasStatusChanged() {
    if (statusChanged) return true;
    
    SystemStatus currentStatus = getCurrentSystemStatus();
    
    // **COMPARACIÓN OPTIMIZADA** - Solo campos críticos
    return (currentStatus.irrigationState != lastKnownStatus.irrigationState ||
            currentStatus.activeZone != lastKnownStatus.activeZone ||
            currentStatus.remainingTime != lastKnownStatus.remainingTime ||
            currentStatus.hasErrors != lastKnownStatus.hasErrors ||
            currentStatus.humidityPercent != lastKnownStatus.humidityPercent);
}

// =============================================================================
// Métodos de utilidad
// =============================================================================

void WebSocketManager::sendHeartbeat() {
    if (!webSocket || webSocket->count() == 0) return;
    
    DynamicJsonDocument doc(128);
    doc["type"] = "heartbeat";
    doc["timestamp"] = millis();
    doc["clients"] = webSocket->count();
    
    String message;
    serializeJson(doc, message);
    
    webSocket->textAll(message);
    VERBOSE_PRINTLN("[WebSocket] Heartbeat enviado");
}

void WebSocketManager::cleanupInactiveConnections() {
    if (!webSocket) return;
    
    // AsyncWebSocket maneja automáticamente la limpieza
    webSocket->cleanupClients();
}

bool WebSocketManager::validateCommand(const String& command, const String& parameters) {
    // **VALIDACIÓN BÁSICA DE COMANDOS**
    if (command.length() == 0) return false;
    
    // Lista de comandos válidos
    const String validCommands[] = {
        "start_irrigation", "stop_irrigation", "emergency_stop",
        "open_zone", "close_zone", "set_zone_time", "enable_zone", "get_status"
    };
    
    for (const String& validCmd : validCommands) {
        if (command == validCmd) return true;
    }
    
    return false;
}

void WebSocketManager::logWebSocketEvent(const String& event, uint32_t clientId) {
    if (SystemDebug::ENABLE_VERBOSE_LOGGING) {
        String logMessage = "[WebSocket] " + event;
        if (clientId > 0) {
            logMessage += " (Cliente ID: " + String(clientId) + ")";
        }
        DEBUG_PRINTLN(logMessage);
    }
}

// =============================================================================
// Métodos públicos de consulta
// =============================================================================

void WebSocketManager::getConnectionStatistics(uint32_t& connectedClients, uint32_t& totalConnections, 
                                               uint32_t& messagesSent, uint32_t& messagesReceived) const {
    connectedClients = webSocket ? webSocket->count() : 0;
    totalConnections = totalConnectionsCount;
    messagesSent = messagesSentCount;
    messagesReceived = messagesReceivedCount;
}

AsyncWebSocket* WebSocketManager::getAsyncWebSocket() const {
    return webSocket;
}

// =============================================================================
// Funciones de utilidad global
// =============================================================================

WebSocketManager* createWebSocketManager(ServoPWMController* controller) {
    WebSocketManager* manager = new WebSocketManager("/ws", controller);
    
    if (!manager->initialize()) {
        delete manager;
        return nullptr;
    }
    
    return manager;
}

const char* messageTypeToString(WSMessageType type) {
    switch(type) {
        case WSMessageType::STATUS_UPDATE: return "status_update";
        case WSMessageType::PARAMETER_CHANGE: return "parameter_change";
        case WSMessageType::COMMAND_EXECUTE: return "command_execute";
        case WSMessageType::ERROR_NOTIFICATION: return "error_notification";
        case WSMessageType::HEARTBEAT: return "heartbeat";
        case WSMessageType::CLIENT_INFO: return "client_info";
        default: return "unknown";
    }
}
