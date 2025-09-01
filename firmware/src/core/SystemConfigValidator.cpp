#include "../../include/core/SystemConfig.h"
#include <Arduino.h>

bool SystemConfigValidator::validateSafetyLimits() {
    // Verificar que los límites de seguridad sean razonables
    return (SystemSafety::MAX_TOTAL_IRRIGATION_TIME_MINUTES > 0 && 
            SystemSafety::MAX_TOTAL_IRRIGATION_TIME_MINUTES <= 1440 && // Máximo 24 horas
            SystemSafety::WATCHDOG_TIMEOUT_MS >= 1000 && // Mínimo 1 segundo
            SystemSafety::MAX_CONSECUTIVE_ERRORS > 0);
}

bool SystemConfigValidator::validateNetworkSettings() {
    // Verificar configuración de red
    return (NetworkConfig::WEB_SERVER_PORT > 0 &&
            NetworkConfig::WIFI_CONNECTION_TIMEOUT_MS >= 5000 &&
            NetworkConfig::MAX_WIFI_RETRY_ATTEMPTS > 0);
}

bool SystemConfigValidator::validateHardwareConfiguration() {
    // Validar configuración de hardware
    bool valid = true;
    
    // Verificar que tenemos pines definidos para todos los servos
    if (HardwareConfig::NUM_SERVOS > 0) {
        for (int i = 0; i < HardwareConfig::NUM_SERVOS; i++) {
            if (HardwareConfig::SERVO_PINS[i] == 0) {
                Serial.println("[CONFIG ERROR] Pin no definido para servo " + String(i));
                valid = false;
            }
        }
    }
    
    // Verificar pines RTC
    if (HardwareConfig::RTC_RST == 0 || HardwareConfig::RTC_SCLK == 0 || HardwareConfig::RTC_IO == 0) {
        Serial.println("[CONFIG ERROR] Pines RTC no definidos correctamente");
        valid = false;
    }
    
    return valid;
}

bool SystemConfigValidator::validateAllConfiguration() {
    bool allValid = true;
    
    // Validar configuración de seguridad
    if (!validateSafetyLimits()) {
        Serial.println("[CONFIG ERROR] Límites de seguridad inválidos");
        allValid = false;
    }
    
    // Validar configuración de red
    if (!validateNetworkSettings()) {
        Serial.println("[CONFIG ERROR] Configuración de red inválida");
        allValid = false;
    }
    
    // Validar configuración de hardware
    if (!validateHardwareConfiguration()) {
        Serial.println("[CONFIG ERROR] Configuración de hardware inválida");
        allValid = false;
    }
    
    if (allValid) {
        Serial.println("[CONFIG OK] Toda la configuración del sistema es válida");
    } else {
        Serial.println("[CONFIG ERROR] Se encontraron errores en la configuración");
    }
    
    return allValid;
}

void SystemConfigValidator::printConfigurationSummary() {
    Serial.println("\n" + String(repeatChar('=', 50)));
    Serial.println("    RESUMEN DE CONFIGURACIÓN DEL SISTEMA");
    Serial.println(String(repeatChar('=', 50)));
    
    Serial.println("🔧 CONFIGURACIÓN DE DEBUG:");
    Serial.println("   • Serial debugging: " + String(SystemDebug::ENABLE_SERIAL_DEBUGGING ? "HABILITADO" : "DESHABILITADO"));
    Serial.println("   • Verbose logging: " + String(SystemDebug::ENABLE_VERBOSE_LOGGING ? "HABILITADO" : "DESHABILITADO"));
    Serial.println("   • Baud rate: " + String(SystemDebug::SERIAL_BAUD_RATE));
    
    Serial.println("\n🛡️ CONFIGURACIÓN DE SEGURIDAD:");
    Serial.println("   • Tiempo máximo riego total: " + String(SystemSafety::MAX_TOTAL_IRRIGATION_TIME_MINUTES) + " minutos");
    Serial.println("   • Timeout watchdog: " + String(SystemSafety::WATCHDOG_TIMEOUT_MS / 1000) + " segundos");
    Serial.println("   • Errores máximos consecutivos: " + String(SystemSafety::MAX_CONSECUTIVE_ERRORS));
    
    Serial.println("\n🌐 CONFIGURACIÓN DE RED:");
    Serial.println("   • Puerto servidor web: " + String(NetworkConfig::WEB_SERVER_PORT));
    Serial.println("   • Timeout conexión WiFi: " + String(NetworkConfig::WIFI_CONNECTION_TIMEOUT_MS / 1000) + " segundos");
    Serial.println("   • Máximo reintentos WiFi: " + String(NetworkConfig::MAX_WIFI_RETRY_ATTEMPTS));
    
    Serial.println("\n🔌 CONFIGURACIÓN OF HARDWARE:");
    Serial.println("   • Número de servos: " + String(HardwareConfig::NUM_SERVOS));
    Serial.println("   • Pines RTC: RST=" + String(HardwareConfig::RTC_RST) + 
                  ", SCLK=" + String(HardwareConfig::RTC_SCLK) + 
                  ", IO=" + String(HardwareConfig::RTC_IO));
    Serial.println("   • LED de estado: Pin " + String(HardwareConfig::LED));
    
    Serial.println(String(repeatChar('=', 50)) + "\n");
}
