#ifndef __SECURE_CONFIG_H__
#define __SECURE_CONFIG_H__

/**
 * @file SecureConfig.h
 * @brief Sistema de configuración WiFi simplificado para ESP32.
 * 
 * Versión simplificada sin almacenamiento persistente para evitar problemas de compatibilidad.
 */

#include <Arduino.h>
#include <WiFi.h>

// =============================================================================
// CONFIGURACIÓN DE SEGURIDAD
// =============================================================================

namespace SecureConfig {
    constexpr uint32_t CONFIG_TIMEOUT_MS = 60000; // Timeout configuración 60s
}

// =============================================================================
// FUNCIÓN DE UTILIDAD PARA SETUP WIFI DIRECTO
// =============================================================================

/**
 * @brief Configuración WiFi directa para debugging.
 */
inline bool setupDirectWiFi() {
    Serial.println("🔌 [DirectWiFi] Configuración WiFi directa...");
    
    // Configuración para red OPTI
    const char* ssid = "OPTI";
    const char* password = "12345678";
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    
    Serial.println("🔌 [DirectWiFi] Conectando a: " + String(ssid));
    WiFi.begin(ssid, password);
    
    unsigned long startTime = millis();
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 30000) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        if (attempts % 20 == 0) {
            Serial.println("");
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ [DirectWiFi] Conectado exitosamente!");
        Serial.println("🌐 IP: " + WiFi.localIP().toString());
        Serial.println("📶 Señal: " + String(WiFi.RSSI()) + " dBm");
        return true;
    } else {
        Serial.println("\n❌ [DirectWiFi] Fallo en conexión WiFi");
        Serial.println("🔍 Estado WiFi: " + String(WiFi.status()));
        
        // Modo AP como fallback
        Serial.println("🔄 [DirectWiFi] Activando modo AP como fallback...");
        WiFi.mode(WIFI_AP);
        
        const char* ap_ssid = "RiegoInteligente_Config";
        const char* ap_password = "configure2025";
        
        if (WiFi.softAP(ap_ssid, ap_password)) {
            Serial.println("✅ [DirectWiFi] Modo AP activo");
            Serial.println("📱 Conectar a: " + String(ap_ssid));
            Serial.println("🔑 Password: " + String(ap_password));
            Serial.println("🌐 Panel: http://192.168.4.1");
            return true;
        }
        
        return false;
    }
}

/**
 * @brief Función de configuración WiFi que siempre usa el método directo.
 */
inline bool setupWiFi() {
    Serial.println("🔄 [WiFi] Iniciando configuración WiFi...");
    return setupDirectWiFi();
}

#endif // __SECURE_CONFIG_H__
