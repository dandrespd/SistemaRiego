#ifndef __WEBSOCKET_MANAGER_H__
#define __WEBSOCKET_MANAGER_H__

/**
 * @file WebSocketManager.h
 * @brief Gestor de WebSockets para comunicación en tiempo real con la interfaz web.
 * 
 * **CONCEPTO EDUCATIVO - COMUNICACIÓN ASÍNCRONA EN TIEMPO REAL**:
 * Como profesores siempre enfatizamos, la diferencia entre polling y WebSockets es
 * fundamental en sistemas modernos. Imagina la diferencia entre:
 * 
 * POLLING (método anterior):
 * - Cliente pregunta cada 2 segundos: "¿Hay cambios?"
 * - Servidor responde: "No" (la mayoría de las veces)
 * - Desperdicio de ancho de banda y recursos
 * - Latencia inherente de hasta 2 segundos
 * 
 * WEBSOCKETS (método optimizado):
 * - Conexión persistente bidireccional
 * - Servidor envía actualizaciones solo cuando hay cambios reales
 * - Latencia mínima (milisegundos)
 * - Uso eficiente de recursos de red y CPU
 * 
 * **ANALOGÍA EDUCATIVA**: Es como la diferencia entre llamar por teléfono cada
 * pocos minutos para preguntar si pasó algo (polling) versus dejar el teléfono
 * conectado y que la otra persona te llame cuando algo suceda (WebSockets).
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.1 - Optimización de comunicación en tiempo real
 * @date 2025
 */

// =============================================================================
// Includes necesarios
// =============================================================================
#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include "ServoPWMController.h"
#include "SystemConfig.h"

// =============================================================================
// Configuración específica de WebSockets
// =============================================================================

namespace WebSocketConfig {
    constexpr uint16_t MAX_CONCURRENT_CLIENTS = 5;         // Máximo 5 clientes simultáneos
    constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000;      // Ping cada 30 segundos
    constexpr uint32_t CLIENT_TIMEOUT_MS = 60000;          // Timeout cliente 60s
    constexpr size_t MAX_MESSAGE_SIZE = 1024;              // Tamaño máximo mensaje JSON
    constexpr uint32_t STATUS_UPDATE_INTERVAL_MS = 1000;   // Updates cada segundo
}

// =============================================================================
// Enumeraciones para tipos de mensajes WebSocket
// =============================================================================

/**
 * @enum WSMessageType
 * @brief Tipos de mensajes WebSocket para comunicación estructurada.
 * 
 * **PATRÓN EDUCATIVO**: Usar enumeraciones para tipos de mensajes previene
 * errores de cadenas mágicas y facilita el mantenimiento. Es como tener
 * un vocabulario formal predefinido para la comunicación.
 */
enum class WSMessageType {
    STATUS_UPDATE,      // Actualización de estado del sistema
    PARAMETER_CHANGE,   // Cambio de parámetros de configuración
    COMMAND_EXECUTE,    // Ejecución de comandos (start/stop/emergency)
    ERROR_NOTIFICATION, // Notificación de errores
    HEARTBEAT,          // Mensaje de verificación de conexión
    CLIENT_INFO         // Información del cliente conectado
};

// =============================================================================
// Estructura de datos para estado del sistema
// =============================================================================

/**
 * @struct SystemStatus
 * @brief Estructura optimizada para envío de estado del sistema via WebSocket.
 * 
 * **CONCEPTO DE OPTIMIZACIÓN**: Esta estructura está diseñada para minimizar
 * el tamaño de los mensajes JSON sin sacrificar información importante.
 * Cada campo tiene un propósito específico y está optimizado para serialización.
 */
struct SystemStatus {
    // Estado del sistema de riego
    String irrigationState;
    uint8_t activeZone;
    uint32_t remainingTime;
    uint32_t totalCycles;
    
    // Estado de sensores
    int humidityPercent;
    int humidityThreshold;
    
    // Estado de zonas individuales
    bool zonesEnabled[NUM_SERVOS];
    uint32_t zoneTimes[NUM_SERVOS];
    
    // Información del sistema
    uint32_t systemUptime;
    uint32_t freeMemory;
    bool hasErrors;
    
    // Timestamp para sincronización
    uint32_t timestamp;
};

// =============================================================================
// Clase Principal: WebSocketManager
// =============================================================================

/**
 * @class WebSocketManager
 * @brief Gestor centralizado de comunicación WebSocket para el sistema de riego.
 * 
 * **PATRÓN DE DISEÑO - MEDIATOR + OBSERVER**:
 * Esta clase actúa como mediador entre el sistema de riego y múltiples clientes web,
 * implementando también el patrón Observer para notificar cambios automáticamente.
 * 
 * **RESPONSABILIDADES**:
 * 1. Gestionar conexiones de múltiples clientes WebSocket
 * 2. Serializar y enviar actualizaciones de estado en tiempo real
 * 3. Procesar comandos entrantes de los clientes
 * 4. Mantener sincronización entre todos los clientes conectados
 * 5. Manejar desconexiones y reconexiones de manera robusta
 * 
 * **BENEFICIOS EDUCATIVOS**:
 * - Demuestra comunicación bidireccional eficiente
 * - Implementa manejo robusto de múltiples clientes
 * - Muestra serialización JSON optimizada
 * - Ejemplifica manejo de errores en comunicación de red
 */
class WebSocketManager {
private:
    // =========================================================================
    // Atributos privados
    // =========================================================================
    
    AsyncWebSocket* webSocket;              // Servidor WebSocket principal
    ServoPWMController* irrigationController; // Referencia al controlador de riego
    
    // Control de actualizaciones y timing
    unsigned long lastStatusUpdate;         // Última actualización de estado enviada
    unsigned long lastHeartbeat;           // Último heartbeat enviado
    
    // Cache de estado para optimización
    SystemStatus lastKnownStatus;          // Último estado conocido
    bool statusChanged;                    // Flag de cambio de estado
    
    // Estadísticas de conexión
    uint32_t totalConnectionsCount;        // Total de conexiones históricas
    uint32_t messagesSentCount;           // Total de mensajes enviados
    uint32_t messagesReceivedCount;       // Total de mensajes recibidos

public:
    // =========================================================================
    // Constructor y métodos públicos principales
    // =========================================================================
    
    /**
     * @brief Constructor del gestor de WebSockets.
     * 
     * @param path Ruta del endpoint WebSocket (ejemplo: "/ws")
     * @param controller Referencia al controlador de riego
     */
    explicit WebSocketManager(const char* path, ServoPWMController* controller);
    
    /**
     * @brief Destructor que limpia recursos de red.
     */
    ~WebSocketManager();
    
    /**
     * @brief Inicializa el servidor WebSocket y configura callbacks.
     * 
     * **CONCEPTO EDUCATIVO**: Los callbacks son funciones que se ejecutan
     * cuando ocurren eventos específicos. Es como programar un sistema de
     * alarma que reacciona a diferentes tipos de situaciones.
     * 
     * @return true si la inicialización fue exitosa
     */
    bool initialize();
    
    /**
     * @brief Función principal de procesamiento (llamar en loop principal).
     * 
     * **PATRÓN NO BLOQUEANTE**: Esta función implementa procesamiento
     * asíncrono que no bloquea otras operaciones del sistema.
     */
    void update();
    
    /**
     * @brief Envía actualización de estado a todos los clientes conectados.
     * 
     * **OPTIMIZACIÓN**: Solo envía si hay cambios reales en el estado,
     * reduciendo tráfico de red innecesario.
     */
    void broadcastStatusUpdate();
    
    /**
     * @brief Envía notificación de error a todos los clientes.
     * 
     * @param errorMessage Mensaje de error descriptivo
     * @param severity Severidad del error (INFO, WARNING, ERROR, CRITICAL)
     */
    void broadcastError(const String& errorMessage, const String& severity = "ERROR");
    
    /**
     * @brief Procesa comando recibido desde un cliente web.
     * 
     * **SEGURIDAD**: Incluye validación de comandos para prevenir
     * operaciones no autorizadas o peligrosas.
     * 
     * @param clientId ID del cliente que envía el comando
     * @param command Comando a ejecutar
     * @param parameters Parámetros del comando en formato JSON
     * @return true si el comando se ejecutó correctamente
     */
    bool processClientCommand(uint32_t clientId, const String& command, const String& parameters);
    
    /**
     * @brief Obtiene estadísticas de conexión para monitoreo.
     * 
     * @param connectedClients Número de clientes actualmente conectados
     * @param totalConnections Total de conexiones desde el inicio
     * @param messagesSent Total de mensajes enviados
     * @param messagesReceived Total de mensajes recibidos
     */
    void getConnectionStatistics(uint32_t& connectedClients, uint32_t& totalConnections, 
                               uint32_t& messagesSent, uint32_t& messagesReceived) const;
    
    /**
     * @brief Obtiene la instancia de AsyncWebSocket para integración con servidor HTTP.
     * 
     * **CONCEPTO DE ENCAPSULACIÓN**: Proporcionamos acceso controlado al
     * WebSocket interno para que pueda ser registrado en el servidor HTTP,
     * pero mantenemos el control de su ciclo de vida.
     * 
     * @return Puntero al AsyncWebSocket interno
     */
    AsyncWebSocket* getAsyncWebSocket() const;
    
    /**
     * @brief Fuerza una actualización de estado (para eventos externos).
     * 
     * Útil cuando otros módulos cambian el estado del sistema y necesitamos
     * notificar inmediatamente a los clientes conectados.
     */
    void forceStatusUpdate();

private:
    // =========================================================================
    // Métodos privados para manejo interno
    // =========================================================================
    
    /**
     * @brief Callback para eventos de WebSocket (conexión, desconexión, mensajes).
     */
    void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                            AwsEventType type, void* arg, uint8_t* data, size_t len);
    
    /**
     * @brief Procesa mensaje recibido de un cliente.
     */
    void handleClientMessage(AsyncWebSocketClient* client, const String& message);
    
    /**
     * @brief Serializa el estado actual del sistema a JSON.
     * 
     * **OPTIMIZACIÓN**: Usa ArduinoJson para serialización eficiente
     * con manejo automático de memoria.
     */
    String serializeSystemStatus();
    
    /**
     * @brief Compara estados para detectar cambios significativos.
     * 
     * **CONCEPTO DE EFICIENCIA**: Evita enviar actualizaciones innecesarias
     * comparando el estado actual con el último estado enviado.
     */
    bool hasStatusChanged();
    
    /**
     * @brief Envía mensaje de heartbeat para mantener conexiones activas.
     * 
     * **CONCEPTO DE REDES**: Los heartbeats previenen timeouts de red
     * y detectan conexiones perdidas de manera proactiva.
     */
    void sendHeartbeat();
    
    /**
     * @brief Limpia conexiones inactivas o perdidas.
     */
    void cleanupInactiveConnections();
    
    /**
     * @brief Obtiene información del sistema para estado actual.
     */
    SystemStatus getCurrentSystemStatus();
    
    /**
     * @brief Valida comando recibido antes de ejecutarlo.
     */
    bool validateCommand(const String& command, const String& parameters);
    
    /**
     * @brief Envía respuesta específica a un cliente.
     */
    void sendResponseToClient(AsyncWebSocketClient* client, const String& response);
    
    /**
     * @brief Registra evento para debugging y auditoría.
     */
    void logWebSocketEvent(const String& event, uint32_t clientId = 0);
};

// =============================================================================
// Funciones de utilidad para integración
// =============================================================================

/**
 * @brief Factory function para crear instancia del WebSocketManager.
 * 
 * **PATRÓN FACTORY**: Encapsula la lógica de creación y configuración
 * del objeto, proporcionando una interfaz simple para su uso.
 */
WebSocketManager* createWebSocketManager(ServoPWMController* controller);

/**
 * @brief Función de utilidad para convertir enumeración a string.
 */
const char* messageTypeToString(WSMessageType type);

#endif // __WEBSOCKET_MANAGER_H__
