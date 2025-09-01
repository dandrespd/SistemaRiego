/**
 * @file WebSocketMessagePool.h
 * @brief Pool de mensajes WebSocket para optimización de memoria y rendimiento.
 * 
 * **CONCEPTO EDUCATIVO - OBJECT POOLING PARA COMUNICACIÓN**:
 * Este sistema reutiliza objetos de mensaje WebSocket para reducir la fragmentación
 * de memoria y mejorar el rendimiento en sistemas embebidos con recursos limitados.
 * 
 * @author Sistema de Riego Inteligente
 * @version 4.0 - Optimización de Memoria
 * @date 2025
 */

#ifndef __WEBSOCKET_MESSAGE_POOL_H__
#define __WEBSOCKET_MESSAGE_POOL_H__

#include <Arduino.h>
#include <WString.h>
#include <ArduinoJson.h>
#include "utils/ObjectPool.h"

/**
 * @brief Estructura para mensajes WebSocket optimizados.
 */
struct WebSocketMessage {
    String type;
    String content;
    uint32_t timestamp;
    uint16_t clientId;
    
    /**
     * @brief Limpia el mensaje para reutilización.
     */
    void clear() {
        type = "";
        content = "";
        timestamp = 0;
        clientId = 0;
    }
    
    /**
     * @brief Serializa el mensaje a JSON.
     * @return String con el JSON serializado
     */
    String toJson() const {
        DynamicJsonDocument doc(512);
        doc["type"] = type;
        doc["content"] = content;
        doc["timestamp"] = timestamp;
        if (clientId > 0) {
            doc["clientId"] = clientId;
        }
        
        String output;
        serializeJson(doc, output);
        return output;
    }
    
    /**
     * @brief Deserializa JSON a mensaje.
     * @param json String con JSON a deserializar
     * @return true si la deserialización fue exitosa
     */
    bool fromJson(const String& json) {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, json);
        
        if (error) {
            return false;
        }
        
        type = doc["type"] | "";
        content = doc["content"] | "";
        timestamp = doc["timestamp"] | millis();
        clientId = doc["clientId"] | 0;
        
        return true;
    }
};

/**
 * @brief Pool de mensajes WebSocket con capacidad predefinida.
 */
class WebSocketMessagePool {
private:
    ObjectPool<WebSocketMessage, 20> messagePool; // Pool de 20 mensajes
    
public:
    /**
     * @brief Obtiene un mensaje del pool.
     * @return Puntero al mensaje o nullptr si el pool está vacío
     */
    WebSocketMessage* acquireMessage() {
        WebSocketMessage* msg = messagePool.acquire();
        if (msg) {
            msg->clear();
        }
        return msg;
    }
    
    /**
     * @brief Libera un mensaje de vuelta al pool.
     * @param msg Mensaje a liberar
     */
    void releaseMessage(WebSocketMessage* msg) {
        if (msg) {
            messagePool.release(msg);
        }
    }
    
    /**
     * @brief Obtiene el número de mensajes disponibles.
     * @return Cantidad de mensajes disponibles
     */
    size_t availableMessages() const {
        return messagePool.availableCount();
    }
    
    /**
     * @brief Obtiene el porcentaje de uso del pool.
     * @return Porcentaje de uso (0-100)
     */
    uint8_t usagePercentage() const {
        return messagePool.usagePercentage();
    }
    
    /**
     * @brief Crea un mensaje de estado del sistema.
     * @param systemUptime Tiempo de actividad del sistema
     * @param freeMemory Memoria libre disponible
     * @param irrigationState Estado del riego
     * @param activeZone Zona activa actual
     * @return Mensaje configurado listo para enviar
     */
    WebSocketMessage* createStatusMessage(uint32_t systemUptime, 
                                         uint32_t freeMemory,
                                         const String& irrigationState,
                                         uint8_t activeZone = 0) {
        WebSocketMessage* msg = acquireMessage();
        if (!msg) {
            return nullptr;
        }
        
        msg->type = "status_update";
        msg->timestamp = millis();
        
        DynamicJsonDocument doc(512);
        doc["systemUptime"] = systemUptime;
        doc["freeMemory"] = freeMemory;
        doc["irrigationState"] = irrigationState;
        if (activeZone > 0) {
            doc["activeZone"] = activeZone;
        }
        
        serializeJson(doc, msg->content);
        return msg;
    }
    
    /**
     * @brief Crea un mensaje de error.
     * @param errorMessage Mensaje de error
     * @param severity Gravedad del error
     * @param clientId ID del cliente (opcional)
     * @return Mensaje configurado listo para enviar
     */
    WebSocketMessage* createErrorMessage(const String& errorMessage, 
                                       const String& severity = "error",
                                       uint16_t clientId = 0) {
        WebSocketMessage* msg = acquireMessage();
        if (!msg) {
            return nullptr;
        }
        
        msg->type = "error";
        msg->timestamp = millis();
        msg->clientId = clientId;
        
        DynamicJsonDocument doc(256);
        doc["message"] = errorMessage;
        doc["severity"] = severity;
        
        serializeJson(doc, msg->content);
        return msg;
    }
    
    /**
     * @brief Crea un mensaje de heartbeat.
     * @param clientCount Número de clientes conectados
     * @return Mensaje configurado listo para enviar
     */
    WebSocketMessage* createHeartbeatMessage(uint16_t clientCount = 0) {
        WebSocketMessage* msg = acquireMessage();
        if (!msg) {
            return nullptr;
        }
        
        msg->type = "heartbeat";
        msg->timestamp = millis();
        
        DynamicJsonDocument doc(128);
        doc["timestamp"] = millis();
        if (clientCount > 0) {
            doc["clients"] = clientCount;
        }
        
        serializeJson(doc, msg->content);
        return msg;
    }
};

#endif // __WEBSOCKET_MESSAGE_POOL_H__
