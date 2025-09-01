/**
 * @file EventBus.h
 * @brief Sistema de bus de eventos para comunicación desacoplada entre módulos.
 * 
 * **CONCEPTO EDUCATIVO - PATRÓN OBSERVER/PUBLISH-SUBSCRIBE**:
 * Este sistema permite que los módulos se comuniquen sin conocerse directamente,
 * mejorando la modularidad y facilitando el testing.
 * 
 * **BENEFICIOS**:
 * 1. Desacoplamiento: Los módulos no necesitan referencias directas entre sí
 * 2. Escalabilidad: Fácil añadir nuevos suscriptores sin modificar publicadores
 * 3. Mantenibilidad: Cambios en un módulo no afectan a otros
 * 4. Testabilidad: Fácil simular eventos para testing
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Arquitectura Modular
 * @date 2025
 */

#ifndef __EVENT_BUS_H__
#define __EVENT_BUS_H__

#include <stdint.h>
#include <string.h>

// Número máximo de suscriptores por tipo de evento
#define MAX_SUBSCRIBERS_PER_EVENT 5

/**
 * @enum EventType
 * @brief Tipos de eventos del sistema de riego
 */
enum class EventType {
    // Eventos del sistema
    SYSTEM_STARTED,
    SYSTEM_STOPPED,
    SYSTEM_ERROR,
    
    // Eventos de riego
    IRRIGATION_STARTED,
    IRRIGATION_STOPPED,
    IRRIGATION_ZONE_CHANGED,
    IRRIGATION_TIME_UPDATED,
    
    // Eventos de red
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    WEB_CLIENT_CONNECTED,
    WEB_CLIENT_DISCONNECTED,
    
    // Eventos de hardware
    RTC_CONFIGURED,
    SERVO_MOVED,
    SENSOR_READING,
    
    // Eventos de configuración
    CONFIG_UPDATED,
    CONFIG_SAVED,
    CONFIG_RESET,
    CONFIG_RESTORED,
    
    // Eventos de usuario
    USER_LOGIN,
    USER_LOGOUT,
    USER_SETTING_CHANGED,
    
    // Total de tipos de eventos
    EVENT_TYPE_COUNT
};

/**
 * @struct EventData
 * @brief Datos adicionales para eventos
 */
struct EventData {
    int intValue;
    float floatValue;
    const char* stringValue;
    void* customData;
};

/**
 * @typedef EventCallback
 * @brief Callback para manejo de eventos
 */
typedef void (*EventCallback)(EventType, const EventData*);

/**
 * @class EventBus
 * @brief Sistema central de bus de eventos
 */
class EventBus {
private:
    // Array de suscriptores por tipo de evento
    EventCallback subscribers[(int)EventType::EVENT_TYPE_COUNT][MAX_SUBSCRIBERS_PER_EVENT];
    uint8_t subscriberCounts[(int)EventType::EVENT_TYPE_COUNT];
    
    // Constructor privado para singleton
    EventBus() {
        memset(subscriberCounts, 0, sizeof(subscriberCounts));
        memset(subscribers, 0, sizeof(subscribers));
    }

public:
    /**
     * @brief Obtener instancia singleton
     */
    static EventBus& getInstance() {
        static EventBus instance;
        return instance;
    }
    
    /**
     * @brief Suscribirse a un tipo de evento
     * @param eventType Tipo de evento
     * @param callback Función callback
     * @return true si la suscripción fue exitosa, false si no hay espacio
     */
    bool subscribe(EventType eventType, EventCallback callback) {
        int index = (int)eventType;
        if (index >= 0 && index < (int)EventType::EVENT_TYPE_COUNT) {
            if (subscriberCounts[index] < MAX_SUBSCRIBERS_PER_EVENT) {
                subscribers[index][subscriberCounts[index]++] = callback;
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Publicar un evento
     * @param eventType Tipo de evento
     * @param data Datos del evento (opcional)
     */
    void publish(EventType eventType, const EventData* data = nullptr) {
        int index = (int)eventType;
        if (index >= 0 && index < (int)EventType::EVENT_TYPE_COUNT) {
            for (uint8_t i = 0; i < subscriberCounts[index]; i++) {
                if (subscribers[index][i]) {
                    subscribers[index][i](eventType, data);
                }
            }
        }
    }
    
    /**
     * @brief Publicar evento simple sin datos
     * @param eventType Tipo de evento
     */
    void publishSimple(EventType eventType) {
        publish(eventType, nullptr);
    }
    
    /**
     * @brief Limpiar todos los suscriptores
     */
    void clear() {
        memset(subscriberCounts, 0, sizeof(subscriberCounts));
        memset(subscribers, 0, sizeof(subscribers));
    }
    
    /**
     * @brief Limpiar suscriptores de un tipo específico
     * @param eventType Tipo de evento a limpiar
     */
    void clear(EventType eventType) {
        int index = (int)eventType;
        if (index >= 0 && index < (int)EventType::EVENT_TYPE_COUNT) {
            subscriberCounts[index] = 0;
            memset(subscribers[index], 0, sizeof(subscribers[index]));
        }
    }
};

#endif // __EVENT_BUS_H__
