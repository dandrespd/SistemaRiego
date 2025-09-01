/**
 * @file WebServerManager.h
 * @brief Gestor especializado del servidor HTTP para el sistema de riego.
 * 
 * **CONCEPTO EDUCATIVO - SEPARACIÓN DE RESPONSABILIDADES**:
 * Esta clase se encarga exclusivamente de la configuración y gestión del
 * servidor HTTP, separando esta responsabilidad del control web general.
 * 
 * **BENEFICIOS**:
 * 1. Mejor organización del código
 * 2. Fácil testing del servidor HTTP
 * 3. Reutilización en otros proyectos
 * 4. Mantenibilidad mejorada
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Arquitectura Modular
 * @date 2025
 */

#ifndef __WEB_SERVER_MANAGER_H__
#define __WEB_SERVER_MANAGER_H__

#include <stdint.h>
#include "../core/SystemConfig.h"
#include <ESPAsyncWebServer.h>

// Forward declarations to avoid compilation issues
class SystemManager;
class ServoPWMController;

/**
 * @class WebServerManager
 * @brief Gestiona el servidor HTTP y endpoints REST
 */
class WebServerManager {
private:
    AsyncWebServer* server;
    SystemManager* systemManager;
    bool isServerRunning;
    
    // Configuración de endpoints
    void setupStaticFiles();
    void setupRESTEndpoints();
    void setupErrorHandling();
    void setupConfigurationEndpoints();
    
    // Autenticación
    bool authenticateRequest(AsyncWebServerRequest *request);
    
public:
    /**
     * @brief Constructor del gestor del servidor web
     * @param port Puerto del servidor HTTP
     */
    WebServerManager(uint16_t port = NetworkConfig::WEB_SERVER_PORT);
    
    /**
     * @brief Destructor
     */
    ~WebServerManager();
    
    /**
     * @brief Inicializar el servidor web
     * @param systemManager Referencia al gestor del sistema
     */
    void initialize(SystemManager* systemManager);
    
    /**
     * @brief Obtener el servidor HTTP
     * @return Referencia al servidor AsyncWebServer
     */
    AsyncWebServer& getServer();
    
    /**
     * @brief Iniciar el servidor web
     */
    void start();
    
    /**
     * @brief Detener el servidor web
     */
    void stop();
    
    /**
     * @brief Obtener estado del servidor
     * @return true si el servidor está activo
     */
    bool isRunning() const { return isServerRunning; }
};

#endif // __WEB_SERVER_MANAGER_H__
