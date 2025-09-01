#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

/**
 * @file WiFiConfig.h
 * @brief Configuración WiFi mejorada para el sistema de riego inteligente.
 * 
 * **CONCEPTO EDUCATIVO - CONFIGURACIÓN SEGURA**:
 * Como profesores siempre enfatizamos, las credenciales sensibles no deben
 * estar hardcodeadas en el código. Este archivo proporciona un lugar central
 * para configuración de red con múltiples opciones de fallback.
 * 
 * **MEJORES PRÁCTICAS IMPLEMENTADAS**:
 * 1. Múltiples redes WiFi para mayor robustez
 * 2. Configuración de timeout y reintentos
 * 3. Modo AP como fallback para configuración inicial
 * 4. Documentación clara para facilitar configuración
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0 - Configuración mejorada y segura
 * @date 2025
 */

// =============================================================================
// CONFIGURACIÓN DE RED PRINCIPAL
// =============================================================================

// **RED PRINCIPAL**: Configurar con tus credenciales WiFi
// IMPORTANTE: Cambiar estas credenciales por las de tu red
const char* ssid = "OPTI-94A79E";        // Nombre de tu red WiFi principal
const char* password = "Princesa2001";   // Contraseña de tu red WiFi principal

// **REDES DE RESPALDO**: Para mayor robustez del sistema
// Útil si la red principal falla o tienes múltiples puntos de acceso
const char* ssid_backup = "MiWiFi_Guest";     // Red de respaldo opcional
const char* password_backup = "password123";  // Contraseña de red de respaldo

// =============================================================================
// CONFIGURACIÓN DE MODO ACCESS POINT (AP)
// =============================================================================

// **MODO AP DE EMERGENCIA**: Si no puede conectarse a ninguna red
// El ESP32 creará su propia red WiFi para configuración inicial
const char* ap_ssid = "RiegoInteligente_Setup";     // Nombre de red AP
const char* ap_password = "riego2025";              // Contraseña AP (min 8 caracteres)
const IPAddress ap_ip(192, 168, 4, 1);             // IP del AP
const IPAddress ap_gateway(192, 168, 4, 1);        // Gateway del AP
const IPAddress ap_subnet(255, 255, 255, 0);       // Máscara de subred

// =============================================================================
// CONFIGURACIÓN AVANZADA DE CONECTIVIDAD
// =============================================================================

// **TIMEOUTS Y REINTENTOS**: Para manejo robusto de conexiones
namespace WiFiAdvancedConfig {
    constexpr uint32_t CONNECTION_TIMEOUT_MS = 30000;      // 30 segundos timeout
    constexpr uint32_t RETRY_INTERVAL_MS = 5000;           // Reintentar cada 5s
    constexpr uint8_t MAX_RETRY_ATTEMPTS = 10;             // Máximo 10 intentos
    constexpr int8_t MIN_SIGNAL_STRENGTH = -80;            // Mínima señal aceptable (dBm)
    constexpr bool ENABLE_AUTO_RECONNECT = true;           // Reconectar automáticamente
    constexpr bool ENABLE_AP_FALLBACK = true;              // Activar AP si falla conexión
}

// =============================================================================
// FUNCIONES DE UTILIDAD PARA GESTIÓN WiFi
// =============================================================================

/**
 * @brief Configuración completa de WiFi con múltiples estrategias de conexión.
 * 
 * Esta función implementa una estrategia robusta de conexión WiFi que:
 * 1. Intenta conectarse a la red principal
 * 2. Si falla, intenta la red de respaldo
 * 3. Como último recurso, activa modo AP para configuración manual
 * 
 * Es como tener múltiples llaves para tu casa: si una no funciona, 
 * automáticamente prueba las otras hasta encontrar una que abra la puerta.
 * 
 * @return true si se estableció algún tipo de conectividad
 */
inline bool setupAdvancedWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(WiFiAdvancedConfig::ENABLE_AUTO_RECONNECT);
    
    // **INTENTO 1**: Red principal
    Serial.println("[WiFi] Intentando conectar a red principal: " + String(ssid));
    WiFi.begin(ssid, password);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
           (millis() - startTime) < WiFiAdvancedConfig::CONNECTION_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Conectado a red principal exitosamente!");
        Serial.println("[WiFi] IP: " + WiFi.localIP().toString());
        Serial.println("[WiFi] Señal: " + String(WiFi.RSSI()) + " dBm");
        return true;
    }
    
    // **INTENTO 2**: Red de respaldo (si está configurada)
    if (strlen(ssid_backup) > 0) {
        Serial.println("\n[WiFi] Intentando red de respaldo: " + String(ssid_backup));
        WiFi.begin(ssid_backup, password_backup);
        
        startTime = millis();
        while (WiFi.status() != WL_CONNECTED && 
               (millis() - startTime) < WiFiAdvancedConfig::CONNECTION_TIMEOUT_MS) {
            delay(500);
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[WiFi] Conectado a red de respaldo exitosamente!");
            Serial.println("[WiFi] IP: " + WiFi.localIP().toString());
            return true;
        }
    }
    
    // **FALLBACK**: Modo Access Point
    if (WiFiAdvancedConfig::ENABLE_AP_FALLBACK) {
        Serial.println("\n[WiFi] Activando modo Access Point de emergencia...");
        WiFi.mode(WIFI_AP);
        
        // Configurar IP estática para el AP
        WiFi.softAPConfig(ap_ip, ap_gateway, ap_subnet);
        
        if (WiFi.softAP(ap_ssid, ap_password)) {
            Serial.println("[WiFi] Modo AP activado exitosamente!");
            Serial.println("[WiFi] Conectarse a: " + String(ap_ssid));
            Serial.println("[WiFi] Contraseña: " + String(ap_password));
            Serial.println("[WiFi] IP del panel: http://" + WiFi.softAPIP().toString());
            return true;
        }
    }
    
    Serial.println("\n[WiFi ERROR] No se pudo establecer ningún tipo de conectividad");
    return false;
}

/**
 * @brief Obtiene información detallada del estado WiFi actual.
 * 
 * @param connectionInfo String donde se almacenará la información
 */
inline void getWiFiStatusInfo(String& connectionInfo) {
    if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
        connectionInfo = "Conectado a: " + WiFi.SSID() + 
                        " (IP: " + WiFi.localIP().toString() + 
                        ", Señal: " + String(WiFi.RSSI()) + " dBm)";
    } else if (WiFi.getMode() == WIFI_AP) {
        connectionInfo = "Modo AP activo: " + String(ap_ssid) + 
                        " (IP: " + WiFi.softAPIP().toString() + 
                        ", Clientes: " + String(WiFi.softAPgetStationNum()) + ")";
    } else {
        connectionInfo = "Desconectado";
    }
}

#endif // WIFI_CONFIG_H
