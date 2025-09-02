#include "../../include/core/SystemConfig.h"
#include <stdio.h> // For printf instead of Serial
#include <string> // For std::string
#include "utils/Utils.h" // For repeatChar function

// Simple logging function replacement
void logMessage(const std::string& message) {
    printf("%s\n", message.c_str());
}

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
                logMessage("[CONFIG ERROR] Pin no definido para servo " + std::to_string(i));
                valid = false;
            }
        }
    }
    
    // Verificar pines RTC
    if (HardwareConfig::RTC_RST == 0 || HardwareConfig::RTC_SCLK == 0 || HardwareConfig::RTC_IO == 0) {
        logMessage("[CONFIG ERROR] Pines RTC no definidos correctamente");
        valid = false;
    }
    
    return valid;
}

bool SystemConfigValidator::validateAllConfiguration() {
    bool allValid = true;
    
    // Validar configuración de seguridad
    if (!validateSafetyLimits()) {
        logMessage("[CONFIG ERROR] Límites de seguridad inválidos");
        allValid = false;
    }
    
    // Validar configuración de red
    if (!validateNetworkSettings()) {
        logMessage("[CONFIG ERROR] Configuración de red inválida");
        allValid = false;
    }
    
    // Validar configuración de hardware
    if (!validateHardwareConfiguration()) {
        logMessage("[CONFIG ERROR] Configuración de hardware inválida");
        allValid = false;
    }
    
    if (allValid) {
        logMessage("[CONFIG OK] Toda la configuración del sistema es válida");
    } else {
        logMessage("[CONFIG ERROR] Se encontraron errores en la configuración");
    }
    
    return allValid;
}

void SystemConfigValidator::printConfigurationSummary() {
    std::string divider(50, '=');
    
    logMessage("\n" + divider);
    logMessage("    RESUMEN DE CONFIGURACIÓN DEL SISTEMA");
    logMessage(divider);
    
    logMessage("🔧 CONFIGURACIÓN DE DEBUG:");
    logMessage(std::string("   • Serial debugging: ") + (SystemDebug::ENABLE_SERIAL_DEBUGGING ? "HABILITADO" : "DESHABILITADO"));
    logMessage(std::string("   • Verbose logging: ") + (SystemDebug::ENABLE_VERBOSE_LOGGING ? "HABILITADO" : "DESHABILITADO"));
    logMessage("   • Baud rate: " + std::to_string(SystemDebug::SERIAL_BAUD_RATE));
    
    logMessage("\n🛡️ CONFIGURACIÓN DE SEGURIDAD:");
    logMessage("   • Tiempo máximo riego total: " + std::to_string(SystemSafety::MAX_TOTAL_IRRIGATION_TIME_MINUTES) + " minutos");
    logMessage("   • Timeout watchdog: " + std::to_string(SystemSafety::WATCHDOG_TIMEOUT_MS / 1000) + " segundos");
    logMessage("   • Errores máximos consecutivos: " + std::to_string(SystemSafety::MAX_CONSECUTIVE_ERRORS));
    
    logMessage("\n🌐 CONFIGURACIÓN DE RED:");
    logMessage("   • Puerto servidor web: " + std::to_string(NetworkConfig::WEB_SERVER_PORT));
    logMessage("   • Timeout conexión WiFi: " + std::to_string(NetworkConfig::WIFI_CONNECTION_TIMEOUT_MS / 1000) + " segundos");
    logMessage("   • Máximo reintentos WiFi: " + std::to_string(NetworkConfig::MAX_WIFI_RETRY_ATTEMPTS));
    
    logMessage("\n🔌 CONFIGURACIÓN OF HARDWARE:");
    logMessage("   • Número de servos: " + std::to_string(HardwareConfig::NUM_SERVOS));
    logMessage("   • Pines RTC: RST=" + std::to_string(HardwareConfig::RTC_RST) +
        ", SCLK=" + std::to_string(HardwareConfig::RTC_SCLK) +
        ", IO=" + std::to_string(HardwareConfig::RTC_IO));
    logMessage("   • LED de estado: Pin " + std::to_string(HardwareConfig::LED));
    
    logMessage(divider + "\n");
}
