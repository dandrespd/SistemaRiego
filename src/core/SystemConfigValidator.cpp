#include "../../include/ProjectConfig.h"
#include "../../include/core/SystemConfigValidator.h"
#include <stdio.h>
#include <string>
#include "../../lib/utils/Utils.h"

// Función de logging simplificada
void logMessage(const std::string& message) {
    printf("%s\n", message.c_str());
}

bool SystemConfigValidator::validateSafetyLimits() {
    // Verificar que los límites de seguridad sean razonables
    return (globalConfig.max_total_irrigation_time_minutes > 0 && 
            globalConfig.max_total_irrigation_time_minutes <= 1440 && // Máximo 24 horas
            globalConfig.watchdog_timeout_ms >= 1000 && // Mínimo 1 segundo
            globalConfig.max_consecutive_errors > 0);
}

bool SystemConfigValidator::validateNetworkSettings() {
    // Verificar configuración de red
    return (globalConfig.web_server_port > 0 &&
            globalConfig.wifi_connection_timeout_ms >= 5000 &&
            globalConfig.max_wifi_retry_attempts > 0);
}

bool SystemConfigValidator::validateHardwareConfiguration() {
    // Validar configuración de hardware
    bool valid = true;
    
    // Verificar pines para servos
    if (globalConfig.num_servos > 0) {
        for (int i = 0; i < globalConfig.num_servos; i++) {
            if (globalConfig.servo_pins[i] == 0) {
                logMessage("[CONFIG ERROR] Pin no definido para servo " + std::to_string(i));
                valid = false;
            }
        }
    }
    
    // Verificar pines RTC
    if (globalConfig.rtc_rst == 0 || globalConfig.rtc_sclk == 0 || globalConfig.rtc_io == 0) {
        logMessage("[CONFIG ERROR] Pines RTC no definidos correctamente");
        valid = false;
    }
    
    return valid;
}

bool SystemConfigValidator::validateAllConfiguration() {
    bool allValid = true;
    
    if (!validateSafetyLimits()) {
        logMessage("[CONFIG ERROR] Límites de seguridad inválidos");
        allValid = false;
    }
    
    if (!validateNetworkSettings()) {
        logMessage("[CONFIG ERROR] Configuración de red inválida");
        allValid = false;
    }
    
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
    logMessage(std::string("   • Serial debugging: ") + (globalConfig.enable_serial_debugging ? "HABILITADO" : "DESHABILITADO"));
    logMessage(std::string("   • Verbose logging: ") + (globalConfig.enable_verbose_logging ? "HABILITADO" : "DESHABILITADO"));
    logMessage("   • Baud rate: " + std::to_string(globalConfig.serial_baud_rate));
    
    logMessage("\n🛡️ CONFIGURACIÓN DE SEGURIDAD:");
    logMessage("   • Tiempo máximo riego total: " + std::to_string(globalConfig.max_total_irrigation_time_minutes) + " minutos");
    logMessage("   • Timeout watchdog: " + std::to_string(globalConfig.watchdog_timeout_ms / 1000) + " segundos");
    logMessage("   • Errores máximos consecutivos: " + std::to_string(globalConfig.max_consecutive_errors));
    
    logMessage("\n🌐 CONFIGURACIÓN DE RED:");
    logMessage("   • Puerto servidor web: " + std::to_string(globalConfig.web_server_port));
    logMessage("   • Timeout conexión WiFi: " + std::to_string(globalConfig.wifi_connection_timeout_ms / 1000) + " segundos");
    logMessage("   • Máximo reintentos WiFi: " + std::to_string(globalConfig.max_wifi_retry_attempts));
    
    logMessage("\n🔌 CONFIGURACIÓN OF HARDWARE:");
    logMessage("   • Número de servos: " + std::to_string(globalConfig.num_servos));
    logMessage("   • Pines RTC: RST=" + std::to_string(globalConfig.rtc_rst) +
        ", SCLK=" + std::to_string(globalConfig.rtc_sclk) +
        ", IO=" + std::to_string(globalConfig.rtc_io));
    logMessage("   • LED de estado: Pin " + std::to_string(globalConfig.led));
    
    logMessage(divider + "\n");
}
