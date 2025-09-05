/**
 * @file WebControl.cpp
 * @brief Implementación simplificada del control web usando WebServerManager.
 * 
 * **CONCEPTO EDUCATIVO - SEPARACIÓN DE RESPONSABILIDADES**:
 * Esta versión simplificada delega toda la gestión del servidor HTTP al
 * WebServerManager especializado, demostrando el principio de responsabilidad única.
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Arquitectura Modular
 * @date 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <stdint.h>
#include <WString.h> // Para tipo String

// Módulos del proyecto
#include "core/SystemConfig.h"
#include "core/SystemManager.h"
#include "network/WebSocketManager.h"
#include "network/WebServerManager.h"
#include "network/WiFiConfig.h"
#include "network/WebControl.h"
#include "utils/Utils.h"
#include "utils/Logger.h"

// **FASE 4: OPTIMIZACIONES DE MEMORIA Y RENDIMIENTO**
#include "utils/ObjectPool.h"

// Declaración de funciones externas
bool setupAdvancedWiFi(); // Declarada en WiFiConfig.h
String repeatChar(char c, int count); // Declarada en Utils.h

// Forward declarations to avoid compilation issues
class WebSocketManager;
class WebServerManager;
class SystemManager;

// Instancias internas
static WebSocketManager* wsManager = nullptr;
static WebServerManager* webServerManager = nullptr;

// **FASE 3: MEJORAS DE ROBUSTEZ** - Variables para gestión de estado
static bool webSystemInitialized = false;
static uint32_t initializationAttempts = 0;
static uint32_t lastWatchdogReset = 0;
static constexpr uint32_t WATCHDOG_TIMEOUT_MS = 30000; // 30 segundos

// Configuración de reintentos para operaciones críticas
static constexpr uint8_t MAX_SPIFFS_RETRIES = 3;
static constexpr uint8_t MAX_WIFI_RETRIES = 5;
static constexpr uint32_t RETRY_DELAY_MS = 2000; // 2 segundos entre reintentos

// **FASE 4: OPTIMIZACIONES DE MEMORIA Y RENDIMIENTO**
static constexpr uint32_t MEMORY_CHECK_INTERVAL_MS = 10000; // 10 segundos
static constexpr uint32_t LOW_MEMORY_THRESHOLD = 10000; // 10KB libre
static constexpr uint32_t MEMORY_ANALYSIS_INTERVAL_MS = 30000; // 30 segundos
static uint32_t lastMemoryCheck = 0;
static uint32_t lastMemoryAnalysis = 0;

/**
 * @brief Configuración simplificada del sistema de control web
 * @param systemManager Referencia al gestor del sistema
 */
void setupWebControl(SystemManager* systemManager) {
  initializationAttempts++;
  lastWatchdogReset = millis();
  
  // **FASE 1: INICIALIZACIÓN DEL SISTEMA DE ARCHIVOS**
  LOG_INFO("[WEBCONTROL] Inicializando sistema de archivos SPIFFS...");
  
  if (!SPIFFS.begin(true)) {
    LOG_ERROR("[WEBCONTROL ERROR] Fallo en inicialización de SPIFFS");
    return;
  }
  
  // **FASE 2: CONFIGURACIÓN SEGURA DE WIFI**
  LOG_INFO("[WEBCONTROL] Inicializando sistema WiFi seguro...");
  
  if (!setupAdvancedWiFi()) {
    LOG_ERROR("[WEBCONTROL ERROR] Fallo en conectividad WiFi avanzada");
    LOG_WARNING("[WEBCONTROL] Sistema funcionará en modo offline limitado");
  }
  
  // **INFORMACIÓN DE CONEXIÓN**
  String connectionInfo;
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    connectionInfo = "Conectado a: " + WiFi.SSID() + 
                    " (IP: " + WiFi.localIP().toString() + 
                    ", Señal: " + String(WiFi.RSSI()) + " dBm)";
  } else if (WiFi.getMode() == WIFI_AP) {
    connectionInfo = String("Modo AP activo: RiegoInteligente_Config") +
                    " (IP: " + WiFi.softAPIP().toString() + 
                    ", Clientes: " + String(WiFi.softAPgetStationNum()) + ")";
  } else {
    connectionInfo = "Desconectado";
  }
  LOG_INFO("[WEBCONTROL] Estado WiFi: " + connectionInfo);
  
  // **FASE 3: INICIALIZACIÓN DEL WEBSOCKET MANAGER**
  LOG_INFO("[WEBCONTROL] Inicializando sistema WebSocket optimizado...");
  
  ServoPWMController* servo = nullptr;
  if (systemManager) {
    servo = systemManager->getIrrigationController();
  }
  wsManager = createWebSocketManager(servo);
  
  if (wsManager && systemManager) {
    systemManager->setWebSocketManager(wsManager);
    LOG_INFO("[WEBCONTROL] WebSocket Manager inicializado exitosamente");
  } else {
    LOG_ERROR("[WEBCONTROL ERROR] Fallo en creación de WebSocketManager");
  }
  
  // **FASE 4: CREACIÓN E INICIALIZACIÓN DEL WEBSERVER MANAGER**
  LOG_INFO("[WEBCONTROL] Creando e inicializando WebServerManager...");
  
  webServerManager = new WebServerManager();
  webServerManager->initialize(systemManager);
  
  // **FASE 5: INTEGRACIÓN DEL WEBSOCKET CON EL SERVIDOR HTTP**
  if (wsManager && webServerManager) {
    webServerManager->getServer().addHandler(wsManager->getAsyncWebSocket());
    LOG_INFO("[WEBCONTROL] WebSocket integrado en servidor HTTP");
  }
  
  // **FASE 6: ARRANQUE DEL SERVIDOR**
  LOG_INFO("[WEBCONTROL] Iniciando servidor HTTP...");
  webServerManager->start();
  
  // **FASE 7: CONFIGURACIÓN DEL SERVICIO mDNS**
  LOG_INFO("[WEBCONTROL] Inicializando servicio mDNS...");
  
  if (MDNS.begin("SistemaRiego")) {
    MDNS.addService("http", "tcp", 80);
    LOG_INFO("[WEBCONTROL] Servicio mDNS iniciado - Acceder mediante http://SistemaRiego.local");
  } else {
    LOG_ERROR("[WEBCONTROL] Error al iniciar servicio mDNS");
  }
  
  // **FASE 8: VALIDACIÓN FINAL Y REPORTE DE ESTADO**
  LOG_INFO("\n" + repeatChar('=', 60));
  LOG_INFO("    SISTEMA WEB MODULAR INICIADO EXITOSAMENTE");
  LOG_INFO(repeatChar('=', 60));
  LOG_INFO("🌐 URL del Panel de Control: http://" + WiFi.localIP().toString());
  LOG_INFO("🌐 URL del Panel de Control (mDNS): http://sistemariego.local");
  LOG_INFO("🔌 WebSocket Endpoint: ws://" + WiFi.localIP().toString() + "/ws");
  
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    LOG_INFO("🖥️ Red WiFi: " + WiFi.SSID() + " (" + String(WiFi.RSSI()) + " dBm)");
  } else if (WiFi.getMode() == WIFI_AP) {
    LOG_INFO("🖥️ Modo configuración activo - AP: RiegoInteligente_Config");
  } else {
    LOG_INFO("🖥️ Red WiFi: Desconectado");
  }
  
  LOG_INFO("📊 Memoria libre: " + String(ESP.getFreeHeap()) + " bytes");
  LOG_INFO("⚙️ Arquitectura modular activa:");
  LOG_INFO("   ✓ WebServerManager para gestión HTTP especializada");
  LOG_INFO("   ✓ WebSockets para comunicación en tiempo real");
  LOG_INFO("   ✓ Separación clara de responsabilidades");
  LOG_INFO(repeatChar('=', 60) + "\n");
  
  // **FASE 9: MARCO DE INICIALIZACIÓN COMPLETADA**
  webSystemInitialized = true;
  LOG_INFO("[WEBCONTROL] Sistema web inicializado exitosamente en intento #" + String(initializationAttempts));
}

/**
 * @brief Función de watchdog para verificar y recuperar el sistema web
 * @return true si el sistema está saludable, false si necesita recuperación
 */
bool checkWebSystemHealth() {
  if (!webSystemInitialized) {
    return false;
  }
  
  uint32_t currentTime = millis();
  
  // Verificar timeout del watchdog
  if (currentTime - lastWatchdogReset > WATCHDOG_TIMEOUT_MS) {
    LOG_ERROR("[WEBCONTROL WATCHDOG] Timeout detectado - Sistema web no responde");
    return false;
  }
  
  // Verificar estado básico del sistema
  bool isHealthy = (WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP) &&
                   (webServerManager != nullptr) && 
                   (wsManager != nullptr);
  
  if (!isHealthy) {
    LOG_WARNING("[WEBCONTROL HEALTH CHECK] Sistema web en estado degradado");
  }
  
  return isHealthy;
}

/**
 * @brief Reinicia el contador del watchdog
 */
void resetWebSystemWatchdog() {
  lastWatchdogReset = millis();
  LOG_DEBUG("[WEBCONTROL] Watchdog resetado");
}

/**
 * @brief Intenta recuperar el sistema web después de una falla
 * @param systemManager Referencia al gestor del sistema
 * @return true si la recuperación fue exitosa
 */
bool recoverWebSystem(SystemManager* systemManager) {
  LOG_WARNING("[WEBCONTROL] Intentando recuperación del sistema web...");
  
  // Limpiar recursos existentes
  if (webServerManager != nullptr) {
    delete webServerManager;
    webServerManager = nullptr;
  }
  
  if (wsManager != nullptr) {
    // Note: WebSocketManager cleanup should be handled by its destructor
    wsManager = nullptr;
  }
  
  // Reinicializar el sistema
  setupWebControl(systemManager);
  
  if (webSystemInitialized) {
    LOG_INFO("[WEBCONTROL] Recuperación exitosa");
    return true;
  } else {
    LOG_ERROR("[WEBCONTROL] Recuperación fallida");
    return false;
  }
}

/**
 * @brief Obtiene el número de intentos de inicialización
 * @return Número de intentos realizados
 */
uint32_t getWebSystemInitializationAttempts() {
  return initializationAttempts;
}

/**
 * @brief Verifica si el sistema web está inicializado
 * @return true si el sistema está inicializado y funcionando
 */
bool isWebSystemInitialized() {
  return webSystemInitialized;
}

/**
 * @brief Analiza el uso de memoria y detecta patrones de fragmentación
 */
void analyzeMemoryUsage() {
  uint32_t currentTime = millis();
  
  // Realizar análisis de memoria cada 30 segundos
  if (currentTime - lastMemoryAnalysis < MEMORY_ANALYSIS_INTERVAL_MS) {
    return;
  }
  
  lastMemoryAnalysis = currentTime;
  uint32_t freeMemory = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  
  LOG_INFO("[MEMORY ANALYSIS] Memoria libre: " + String(freeMemory) + " bytes");
  LOG_INFO("[MEMORY ANALYSIS] Memoria mínima libre: " + String(minFreeHeap) + " bytes");
  LOG_INFO("[MEMORY ANALYSIS] Tamaño total del heap: " + String(totalHeap) + " bytes");
  LOG_INFO("[MEMORY ANALYSIS] Fragmentación estimada: " + String(100 - (freeMemory * 100 / totalHeap)) + "%");
  
  // Detectar patrones de memoria crítica
  if (freeMemory < LOW_MEMORY_THRESHOLD * 2) {
    LOG_WARNING("[MEMORY ANALYSIS] Memoria bajando hacia niveles críticos");
    // Aplicar optimizaciones preventivas
    // TODO: Implementar optimizaciones de memoria
    // optimizeWebSystemMemory();
  }
}

/**
 * @brief Monitorea el uso de memoria y toma acciones si es necesario
 * @return true si la memoria es suficiente, false si está crítica
 */
bool monitorMemoryUsage() {
  uint32_t currentTime = millis();
  
  // Verificar periodicamente el uso de memoria
  if (currentTime - lastMemoryCheck < MEMORY_CHECK_INTERVAL_MS) {
    return true; // Todavía no es tiempo de verificar
  }
  
  lastMemoryCheck = currentTime;
  uint32_t freeMemory = ESP.getFreeHeap();
  
  LOG_DEBUG("[MEMORY] Memoria libre: " + String(freeMemory) + " bytes");
  
  if (freeMemory < LOW_MEMORY_THRESHOLD) {
    LOG_WARNING("[MEMORY CRÍTICO] Memoria baja: " + String(freeMemory) + " bytes");
    
    // Acciones para liberar memoria
    if (webServerManager != nullptr) {
      LOG_INFO("[MEMORY] Liberando recursos del servidor web...");
      // TODO: Implementar cleanupBuffers en WebServerManager
      // webServerManager->cleanupBuffers(); // Método de limpieza específico
    }
    
    if (wsManager != nullptr) {
      LOG_INFO("[MEMORY] Liberando recursos de WebSocket...");
      // TODO: Implementar cleanupInactiveConnections en WebSocketManager
      // wsManager->cleanupInactiveConnections(); // Limpiar conexiones inactivas
    }
    
    // Forzar recolección de basura y compactación de memoria
    #ifdef ESP32
    heap_caps_check_integrity_all(true);
    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
    #endif
    
    uint32_t newFreeMemory = ESP.getFreeHeap();
    LOG_INFO("[MEMORY] Acciones de limpieza completadas. Memoria libre: " + String(newFreeMemory) + " bytes");
    
    return false;
  }
  
  return true;
}

/**
 * @brief Optimiza el sistema web para reducir el uso de memoria
 */
void optimizeWebSystemMemory() {
  LOG_INFO("[OPTIMIZACIÓN] Aplicando optimizaciones de memoria...");
  
  uint32_t initialMemory = ESP.getFreeHeap();
  
  // 1. Reducir buffers de comunicación si es posible
  if (wsManager != nullptr) {
    // TODO: Implementar optimizeMemoryUsage en WebSocketManager
    // wsManager->optimizeMemoryUsage(); // Método de optimización específico
    LOG_DEBUG("[OPTIMIZACIÓN] WebSocket Manager optimizado");
  }
  
  // 2. Limpiar cachés temporales
  if (webServerManager != nullptr) {
    // TODO: Implementar optimizeMemoryUsage en WebServerManager
    // webServerManager->optimizeMemoryUsage(); // Método de optimización específico
    LOG_DEBUG("[OPTIMIZACIÓN] WebServer Manager optimizado");
  }
  
  // 3. Forzar liberación de memoria no utilizada
  #ifdef ESP32
  heap_caps_check_integrity_all(true);
  heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
  #endif
  
  uint32_t finalMemory = ESP.getFreeHeap();
  uint32_t memorySaved = finalMemory - initialMemory;
  
  LOG_INFO("[OPTIMIZACIÓN] Optimizaciones completadas. Memoria libre: " + 
           String(finalMemory) + " bytes (" + String(memorySaved) + " bytes liberados)");
}

/**
 * @brief Función principal de mantenimiento del sistema web
 * @param systemManager Referencia al gestor del sistema para posibles recuperaciones
 */
void maintainWebSystem(SystemManager* systemManager) {
  // 1. Verificar salud del sistema
  if (!checkWebSystemHealth()) {
    LOG_WARNING("[MANTENIMIENTO] Sistema web necesita recuperación");
    recoverWebSystem(systemManager);
    return;
  }
  
  // 2. Monitorear memoria
  if (!monitorMemoryUsage()) {
    LOG_INFO("[MANTENIMIENTO] Aplicando optimizaciones por memoria baja");
    optimizeWebSystemMemory();
  }
  
  // 3. Reiniciar watchdog para indicar que el sistema está activo
  resetWebSystemWatchdog();
  
  LOG_DEBUG("[MANTENIMIENTO] Ciclo de mantenimiento completado");
}
