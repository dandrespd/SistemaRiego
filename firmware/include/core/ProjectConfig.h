#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

/**
 * @file ProjectConfig.h
 * @brief Configuración centralizada y consolidada para todo el proyecto de riego inteligente.
 * 
 * **CONCEPTO EDUCATIVO - PRINCIPIO DE RESPONSABILIDAD ÚNICA**:
 * Este archivo actúa como el "panel de control maestro" del sistema, centralizando
 * toda la configuración en un solo lugar. Es como tener un archivo de configuración
 * único que controla todos los aspectos del sistema, facilitando el mantenimiento
 * y evitando inconsistencias.
 * 
 * **PRINCIPIOS DE DISEÑO APLICADOS**:
 * 1. **Configuración Centralizada**: Un solo lugar para todos los parámetros
 * 2. **Modularidad**: Organización por módulos funcionales
 * 3. **Validación Automática**: Verificación de configuración al inicio
 * 4. **Documentación Integrada**: Cada parámetro está documentado
 * 5. **Escalabilidad**: Fácil agregar nuevos parámetros
 * 6. **Robustez**: Validaciones y valores por defecto seguros
 * 
 * **BENEFICIOS DE ESTA ARQUITECTURA**:
 * - Configuración unificada y fácil de mantener
 * - Validación automática de parámetros
 * - Documentación integrada con el código
 * - Fácil debugging y troubleshooting
 * - Escalabilidad para futuras funcionalidades
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0 - Configuración Centralizada
 * @date 2025
 */

// =============================================================================
// Includes fundamentales
// =============================================================================

#include <cstdint>  // For uint8_t, uint32_t etc
#include <string>   // For std::string
#include <cstdio>   // For printf

// Serial interface abstraction
class SerialInterface {
public:
    virtual void println(const char*) = 0;
    virtual void print(const char*) = 0;
    virtual void println(const std::string&) = 0;
    virtual void print(const std::string&) = 0;
};

extern SerialInterface& Serial;  // Global serial interface reference

// =============================================================================
// CONFIGURACIÓN GENERAL DEL PROYECTO
// =============================================================================

/**
 * @brief Información básica del proyecto y versión.
 */
namespace ProjectInfo {
    constexpr const char* PROJECT_NAME = "Sistema de Riego Inteligente";
    constexpr const char* VERSION = "3.2.0";
    constexpr const char* BUILD_DATE = __DATE__;
    constexpr const char* BUILD_TIME = __TIME__;
    constexpr const char* AUTHOR = "Sistema de Riego Inteligente";
    constexpr const char* DESCRIPTION = "Sistema IoT para riego automático multi-zona";
}

// =============================================================================
// CONFIGURACIÓN DE HARDWARE - PINES GPIO
// =============================================================================

/**
 * @brief Configuración centralizada de todos los pines GPIO del sistema.
 * 
 * **PRINCIPIO DE ORGANIZACIÓN**: Los pines están organizados por funcionalidad
 * para facilitar la comprensión y mantenimiento. Cada sección incluye
 * documentación sobre el propósito y restricciones de los pines.
 */
namespace HardwarePins {
    
    // =====================================================================
    // PINES DEL RTC (DS1302) - Real Time Clock
    // =====================================================================
    namespace RTC {
        constexpr uint8_t RST = 17;     // Pin RST (Reset) del módulo RTC DS1302
        constexpr uint8_t SCLK = 18;    // Pin SCLK (Serial Clock) del módulo RTC
        constexpr uint8_t IO = 19;      // Pin IO (Input/Output Data) del módulo RTC
        
        // Array para facilitar iteración
        constexpr uint8_t PINS[] = {RST, SCLK, IO};
        constexpr uint8_t COUNT = sizeof(PINS) / sizeof(PINS[0]);
    }
    
    // =====================================================================
    // LED DE ESTADO DEL SISTEMA
    // =====================================================================
    namespace StatusLED {
        constexpr uint8_t MAIN = 16;    // LED externo para indicadores de estado
                                        // - Parpadeo lento: funcionamiento normal
                                        // - Parpadeo rápido: error del sistema
                                        // - Encendido fijo: modo configuración
    }
    
    // =====================================================================
    // ENTRADAS Y SALIDAS DIGITALES BÁSICAS
    // =====================================================================
    namespace DigitalIO {
        constexpr uint8_t SENSOR_HUMEDAD = 37;  // Entrada digital para sensor de humedad básico
        constexpr uint8_t VALVULA_PRINCIPAL = 21; // Salida digital para control de válvula principal
        
        // Array para facilitar iteración
        constexpr uint8_t INPUTS[] = {SENSOR_HUMEDAD};
        constexpr uint8_t OUTPUTS[] = {VALVULA_PRINCIPAL};
        constexpr uint8_t INPUT_COUNT = sizeof(INPUTS) / sizeof(INPUTS[0]);
        constexpr uint8_t OUTPUT_COUNT = sizeof(OUTPUTS) / sizeof(OUTPUTS[0]);
    }
    
    // =====================================================================
    // PINES DE SERVOMOTORES PWM (Sistema de Riego Multi-Zona)
    // =====================================================================
    namespace Servos {
        // IMPORTANTE: Estos pines deben ser capaces de generar señales PWM
        // Los pines GPIO válidos para PWM en ESP32 son: 0,1,2,3,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33
        
        // Cada servo controla una válvula de zona específica del sistema de riego:
        // - Posición 0°: Válvula cerrada (sin flujo de agua)
        // - Posición configurada: Válvula abierta (flujo de agua hacia la zona)
        constexpr uint8_t ZONA_1 = 25;  // Servo válvula zona 1 (jardín frontal)
        constexpr uint8_t ZONA_2 = 26;  // Servo válvula zona 2 (jardín lateral)
        constexpr uint8_t ZONA_3 = 27;  // Servo válvula zona 3 (huerta trasera)
        constexpr uint8_t ZONA_4 = 32;  // Servo válvula zona 4 (árboles frutales)
        constexpr uint8_t ZONA_5 = 33;  // Servo válvula zona 5 (césped principal)
        
        // Array de pines de servos para facilitar iteración programática
        // Esto nos permite agregar o quitar servos fácilmente modificando solo este array
        // y el sistema automáticamente se adapta al nuevo número de zonas
        constexpr uint8_t PINS[] = {
            ZONA_1, ZONA_2, ZONA_3, ZONA_4, ZONA_5
        };
        
        // Número total de servos/zonas disponibles (calculado automáticamente)
        // Este valor se actualiza automáticamente cuando se modifica el array PINS
        constexpr uint8_t COUNT = sizeof(PINS) / sizeof(PINS[0]);
        
        // Nombres descriptivos de las zonas
        constexpr const char* ZONE_NAMES[] = {
            "Jardín Frontal",
            "Jardín Lateral", 
            "Huerta Trasera",
            "Árboles Frutales",
            "Césped Principal"
        };
    }
    
    // =====================================================================
    // PINES RESERVADOS PARA EXPANSIONES FUTURAS
    // =====================================================================
    namespace Reserved {
        // Los siguientes pines están disponibles para futuras expansiones:
        // GPIO 0, 2, 4, 5, 12, 13, 14, 15, 22, 23
        
        // NOTA: Algunos pines tienen restricciones especiales:
        // - GPIO 0: Usado durante el boot, requiere pull-up
        // - GPIO 2: LED interno, usado durante el boot
        // - GPIO 12: Afecta el voltaje flash durante el boot si está en HIGH
        // - GPIO 15: Pull-up interno durante el boot
        
        // Para sensores analógicos usar: GPIO 32, 33, 34, 35, 36, 37, 38, 39
        // (GPIO 34-39 son solo de entrada y no tienen pull-up/pull-down internos)
        
        constexpr uint8_t AVAILABLE_PINS[] = {0, 2, 4, 5, 12, 13, 14, 15, 22, 23};
        constexpr uint8_t AVAILABLE_COUNT = sizeof(AVAILABLE_PINS) / sizeof(AVAILABLE_PINS[0]);
    }
}

// =============================================================================
// CONFIGURACIÓN DE SERVOMOTORES Y RIEGO
// =============================================================================

/**
 * @brief Configuración detallada del sistema de servomotores y riego.
 */
namespace ServoConfig {
    
    // =====================================================================
    // CONFIGURACIÓN DE ÁNGULOS DE SERVOMOTORES
    // =====================================================================
    namespace Angles {
        constexpr uint8_t CLOSED = 0;    // Ángulo de posición cerrada (válvula cerrada - sin flujo de agua)
        constexpr uint8_t OPEN = 88;     // Ángulo de posición abierta (válvula abierta - flujo máximo de agua)
        
        // Ángulos personalizados por zona (si es necesario)
        constexpr uint8_t ZONE_ANGLES[] = {90, 75, 90, 60, 90}; // Corresponden a las 5 zonas
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE TIEMPOS DE RIEGO
    // =====================================================================
    namespace Timing {
        constexpr uint32_t DEFAULT_IRRIGATION_TIME = 300;  // 5 minutos por zona por defecto
        constexpr uint32_t TRANSITION_TIME = 10;           // 10 segundos de pausa entre zonas
        constexpr uint32_t SERVO_MOVEMENT_TIME = 1000;     // 1 segundo para abrir/cerrar
        
        // Tiempos personalizados por zona (en segundos)
        constexpr uint32_t ZONE_TIMES[] = {300, 240, 360, 180, 420}; // Corresponden a las 5 zonas
        
        // Límites de seguridad
        constexpr uint32_t MAX_IRRIGATION_TIME = 1800;     // 30 minutos máximo por zona
        constexpr uint32_t MIN_IRRIGATION_TIME = 60;       // 1 minuto mínimo por zona
        constexpr uint32_t MAX_TRANSITION_TIME = 300;      // 5 minutos máximo entre zonas
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE PWM (Modulación por Ancho de Pulso)
    // =====================================================================
    namespace PWM {
        constexpr uint32_t FREQUENCY = 50;        // Frecuencia de la señal PWM para servomotores (Hz)
        constexpr uint8_t RESOLUTION = 12;        // Resolución del PWM en bits (4096 pasos)
        
        // Valores de pulso para control de servomotores
        // Los servos estándar usan pulsos de 1-2ms dentro de un período de 20ms
        // Para 50Hz y 12 bits de resolución (4096 pasos):
        // - 1ms = 4096 * (1/20) = 204.8 ≈ 205 pasos
        // - 2ms = 4096 * (2/20) = 409.6 ≈ 410 pasos
        constexpr uint32_t MIN_PULSE = 205;       // ~1ms (0°)
        constexpr uint32_t MAX_PULSE = 410;       // ~2ms (180°)
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE SEGURIDAD Y LÍMITES
    // =====================================================================
    namespace Safety {
        constexpr uint8_t SERVO_RETRY_ATTEMPTS = 3;        // Número máximo de intentos de reposicionamiento de servos
        constexpr uint32_t SERVO_STATUS_INTERVAL = 5000;   // Intervalo de reporte de estado de servos en ms
        constexpr bool ENABLE_SERVO_VERBOSE = true;        // Habilitar modo de diagnóstico detallado para servos
        constexpr bool ENABLE_POSITION_FEEDBACK = false;   // Habilitar verificación de posición
    }
}

// =============================================================================
// CONFIGURACIÓN DE RED Y COMUNICACIÓN
// =============================================================================

/**
 * @brief Configuración de red, WiFi y comunicación del sistema.
 */
namespace NetworkConfig {
    
    // =====================================================================
    // CONFIGURACIÓN DE SERVIDOR WEB
    // =====================================================================
    namespace WebServer {
        constexpr uint16_t PORT = 80;                      // Puerto del servidor web
        constexpr uint32_t REQUEST_TIMEOUT = 5000;         // Timeout de peticiones web en ms
        constexpr uint8_t MAX_CLIENTS = 5;                 // Máximo número de clientes simultáneos
        constexpr bool ENABLE_WEBSOCKETS = true;           // Habilitar WebSockets
        constexpr bool ENABLE_REST_API = true;             // Habilitar API REST (legacy)
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE WIFI
    // =====================================================================
    namespace WiFi {
        constexpr uint32_t CONNECTION_TIMEOUT = 30000;     // 30 segundos timeout de conexión
        constexpr uint32_t RETRY_INTERVAL = 5000;          // Reintentar cada 5 segundos
        constexpr uint8_t MAX_RETRY_ATTEMPTS = 10;         // Máximo 10 reintentos
        constexpr int8_t MIN_SIGNAL_STRENGTH = -80;        // Mínima señal aceptable (dBm)
        constexpr bool ENABLE_AUTO_RECONNECT = true;       // Reconectar automáticamente
        constexpr bool ENABLE_AP_FALLBACK = true;          // Activar AP si falla conexión
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE ACCESS POINT
    // =====================================================================
    namespace AccessPoint {
        constexpr const char* SSID = "RiegoInteligente_Config";  // Nombre de red AP
        constexpr const char* PASSWORD = "configure2025";        // Contraseña AP (min 8 caracteres)
        constexpr uint8_t IP_OCTET_1 = 192;                      // IP del AP: 192.168.4.1
        constexpr uint8_t IP_OCTET_2 = 168;
        constexpr uint8_t IP_OCTET_3 = 4;
        constexpr uint8_t IP_OCTET_4 = 1;
    }
}

// =============================================================================
// CONFIGURACIÓN DE SEGURIDAD
// =============================================================================

/**
 * @brief Configuración de seguridad y credenciales del sistema.
 */
namespace SecurityConfig {
    
    // =====================================================================
    // CONFIGURACIÓN DE EEPROM
    // =====================================================================
    namespace EEPROM {
        constexpr size_t SIZE = 512;                        // Tamaño de EEPROM en bytes
        constexpr size_t MAX_SSID_LENGTH = 32;              // Longitud máxima de SSID
        constexpr size_t MAX_PASSWORD_LENGTH = 64;          // Longitud máxima de password
        constexpr uint32_t MAGIC_NUMBER = 0xDEADBEEF;       // Magic number para validación
        constexpr uint32_t CONFIG_TIMEOUT = 60000;          // Timeout configuración 60s
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE ENCRIPTACIÓN
    // =====================================================================
    namespace Encryption {
        constexpr const char* XOR_KEY = "RiegoSeguro2025";  // Clave para encriptación XOR
        constexpr bool ENABLE_CHECKSUM = true;              // Habilitar verificación de checksum
    }
}

// =============================================================================
// CONFIGURACIÓN DE SENSORES
// =============================================================================

/**
 * @brief Configuración de sensores y monitoreo del sistema.
 */
namespace SensorConfig {
    
    // =====================================================================
    // CONFIGURACIÓN DE SENSOR DE HUMEDAD
    // =====================================================================
    namespace Humidity {
        constexpr uint8_t DEFAULT_THRESHOLD = 40;           // Umbral de humedad por defecto (%)
        constexpr uint8_t MIN_THRESHOLD = 0;                // Umbral mínimo (%)
        constexpr uint8_t MAX_THRESHOLD = 100;              // Umbral máximo (%)
        constexpr uint32_t READ_INTERVAL = 5000;            // Intervalo de lectura en ms
        constexpr bool ENABLE_AUTO_IRRIGATION = true;       // Habilitar riego automático basado en humedad
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE MONITOREO DEL SISTEMA
    // =====================================================================
    namespace Monitoring {
        constexpr uint32_t MEMORY_CHECK_INTERVAL = 30000;   // Verificar memoria cada 30 segundos
        constexpr uint32_t SYSTEM_STATUS_INTERVAL = 60000;  // Reporte de estado del sistema cada 60 segundos
        constexpr uint32_t CRITICAL_MEMORY_THRESHOLD = 10000; // Memoria crítica en bytes
        constexpr uint32_t MIN_MEMORY_THRESHOLD = 5000;     // Memoria mínima antes de reinicio
    }
}

// =============================================================================
// CONFIGURACIÓN DE DEBUGGING Y LOGGING
// =============================================================================

/**
 * @brief Configuración de debugging, logging y monitoreo del sistema.
 */
namespace DebugConfig {
    
    // =====================================================================
    // CONFIGURACIÓN DE COMUNICACIÓN SERIE
    // =====================================================================
    namespace Serial {
        constexpr uint32_t BAUD_RATE = 115200;              // Velocidad de comunicación serie
        constexpr bool ENABLE_DEBUGGING = true;             // Habilitar mensajes de debug
        constexpr bool ENABLE_VERBOSE_LOGGING = true;       // Habilitar logging detallado
    }
    
    // =====================================================================
    // CONFIGURACIÓN DE MONITOREO DE RENDIMIENTO
    // =====================================================================
    namespace Performance {
        constexpr bool ENABLE_PERFORMANCE_MONITORING = false; // Monitoreo de rendimiento
        constexpr bool ENABLE_MEMORY_MONITORING = false;      // Monitoreo de memoria
        constexpr uint32_t LOOP_TIMEOUT_THRESHOLD = 10000;    // Timeout de loop en microsegundos
    }
}

// =============================================================================
// CONFIGURACIÓN DE SEGURIDAD DEL SISTEMA
// =============================================================================

/**
 * @brief Configuración de seguridad y límites críticos del sistema.
 */
namespace SystemSafety {
    constexpr uint32_t MAX_TOTAL_IRRIGATION_TIME = 180;     // 3 horas máximo total de riego
    constexpr uint32_t MIN_PRESSURE_STABILIZATION = 2000;   // Estabilización presión en ms
    constexpr uint32_t WATCHDOG_TIMEOUT = 30000;            // Timeout watchdog 30s
    constexpr uint8_t MAX_CONSECUTIVE_ERRORS = 5;           // Errores antes de parada
    constexpr uint32_t EMERGENCY_STOP_TIMEOUT = 1000;       // Timeout parada emergencia
}

// =============================================================================
// VALIDACIÓN AUTOMÁTICA DE CONFIGURACIÓN
// =============================================================================

/**
 * @class ProjectConfigValidator
 * @brief Clase utilitaria para validar toda la configuración del proyecto.
 * 
 * **PATRÓN EDUCATIVO**: Esta clase implementa el patrón "Validator" que
 * separa la lógica de validación de la lógica de negocio. Es como tener
 * un inspector de calidad independiente que verifica que todo esté correcto
 * antes de que el sistema comience a operar.
 */
class ProjectConfigValidator {
public:
    /**
     * @brief Valida toda la configuración del proyecto.
     * 
     * Esta función actúa como un "preflight check" similar a lo que hacen
     * los pilotos antes del despegue. Verifica que todos los parámetros
     * estén dentro de rangos seguros y sean coherentes entre sí.
     * 
     * @return true si toda la configuración es válida, false en caso contrario
     */
    // Función de utilidad para repetir caracteres
    static std::string repeatChar(char c, int times) {
        return std::string(times, c);
    }
    
    static bool validateAllConfiguration() {
        bool allValid = true;
        
        // Validar configuración de hardware
        if (!validateHardwareConfiguration()) {
            Serial.println("[CONFIG ERROR] Configuración de hardware inválida");
            allValid = false;
        }
        
        // Validar configuración de servomotores
        if (!validateServoConfiguration()) {
            Serial.println("[CONFIG ERROR] Configuración de servomotores inválida");
            allValid = false;
        }
        
        // Validar configuración de red
        if (!validateNetworkConfiguration()) {
            Serial.println("[CONFIG ERROR] Configuración de red inválida");
            allValid = false;
        }
        
        // Validar configuración de seguridad
        if (!validateSecurityConfiguration()) {
            Serial.println("[CONFIG ERROR] Configuración de seguridad inválida");
            allValid = false;
        }
        
        // Validar configuración de sensores
        if (!validateSensorConfiguration()) {
            Serial.println("[CONFIG ERROR] Configuración de sensores inválida");
            allValid = false;
        }
        
        if (allValid) {
            Serial.println("[CONFIG OK] Toda la configuración del proyecto es válida");
        } else {
            Serial.println("[CONFIG ERROR] Se encontraron errores en la configuración");
        }
        
        return allValid;
    }
    
    /**
     * @brief Imprime un resumen completo de la configuración actual.
     * 
     * **CONCEPTO EDUCATIVO**: La transparencia en la configuración es crucial
     * para el debugging y mantenimiento. Esta función proporciona visibilidad
     * completa de cómo está configurado el sistema.
     */
    static void printConfigurationSummary() {
        Serial.println(("\n" + repeatChar('=', 60)).c_str());
        Serial.println("    RESUMEN DE CONFIGURACIÓN DEL PROYECTO");
        Serial.println(repeatChar('=', 60).c_str());
        
        // Información del proyecto
        Serial.println("📋 INFORMACIÓN DEL PROYECTO:");
        Serial.println(("   • Nombre: " + std::string(ProjectInfo::PROJECT_NAME)).c_str());
        Serial.println(("   • Versión: " + std::string(ProjectInfo::VERSION)).c_str());
        Serial.println(("   • Compilado: " + std::string(ProjectInfo::BUILD_DATE) + " " + std::string(ProjectInfo::BUILD_TIME)).c_str());
        
        // Configuración de hardware
        Serial.println("\n🔧 CONFIGURACIÓN DE HARDWARE:");
        Serial.println(("   • Pines RTC: " + std::to_string(HardwarePins::RTC::RST) + ", " + 
                      std::to_string(HardwarePins::RTC::SCLK) + ", " + std::to_string(HardwarePins::RTC::IO)).c_str());
        Serial.println(("   • LED de estado: GPIO " + std::to_string(HardwarePins::StatusLED::MAIN)).c_str());
        Serial.println(("   • Servomotores: " + std::to_string(HardwarePins::Servos::COUNT) + " zonas configuradas").c_str());
        Serial.println(("   • Pines servos: " + std::to_string(HardwarePins::Servos::PINS[0]) + ", " + 
                      std::to_string(HardwarePins::Servos::PINS[1]) + ", " + std::to_string(HardwarePins::Servos::PINS[2]) + ", " +
                      std::to_string(HardwarePins::Servos::PINS[3]) + ", " + std::to_string(HardwarePins::Servos::PINS[4])).c_str());
        
        // Configuración de riego
        Serial.println("\n💧 CONFIGURACIÓN DE RIEGO:");
        Serial.println(("   • Tiempo por zona: " + std::to_string(ServoConfig::Timing::DEFAULT_IRRIGATION_TIME) + " segundos").c_str());
        Serial.println(("   • Tiempo transición: " + std::to_string(ServoConfig::Timing::TRANSITION_TIME) + " segundos").c_str());
        Serial.println(("   • Ángulo cerrado: " + std::to_string(ServoConfig::Angles::CLOSED) + "°").c_str());
        Serial.println(("   • Ángulo abierto: " + std::to_string(ServoConfig::Angles::OPEN) + "°").c_str());
        
        // Configuración de red
        Serial.println("\n🌐 CONFIGURACIÓN DE RED:");
        Serial.println(("   • Puerto servidor web: " + std::to_string(NetworkConfig::WebServer::PORT)).c_str());
        Serial.println(("   • Timeout conexión WiFi: " + std::to_string(NetworkConfig::WiFi::CONNECTION_TIMEOUT / 1000) + " segundos").c_str());
        Serial.println(("   • WebSockets habilitados: " + std::string(NetworkConfig::WebServer::ENABLE_WEBSOCKETS ? "SÍ" : "NO")).c_str());
        
        // Configuración de seguridad
        Serial.println("\n🛡️ CONFIGURACIÓN DE SEGURIDAD:");
        Serial.println(("   • Tamaño EEPROM: " + std::to_string(SecurityConfig::EEPROM::SIZE) + " bytes").c_str());
        Serial.println(("   • Errores máximos consecutivos: " + std::to_string(SystemSafety::MAX_CONSECUTIVE_ERRORS)).c_str());
        Serial.println(("   • Timeout watchdog: " + std::to_string(SystemSafety::WATCHDOG_TIMEOUT / 1000) + " segundos").c_str());
        
        // Configuración de sensores
        Serial.println("\n📊 CONFIGURACIÓN DE SENSORES:");
        Serial.println(("   • Umbral humedad: " + std::to_string(SensorConfig::Humidity::DEFAULT_THRESHOLD) + "%").c_str());
        Serial.println(("   • Riego automático: " + std::string(SensorConfig::Humidity::ENABLE_AUTO_IRRIGATION ? "HABILITADO" : "DESHABILITADO")).c_str());
        
        Serial.println((repeatChar('=', 60) + "\n").c_str());
    }

private:
    static bool validateHardwareConfiguration() {
        // Verificar que no hay conflictos de pines
        // Verificar que los pines de servos son válidos para PWM
        // Verificar que los pines no están duplicados
        
        // Verificar pines PWM válidos para servos
        for (uint8_t i = 0; i < HardwarePins::Servos::COUNT; i++) {
            uint8_t pin = HardwarePins::Servos::PINS[i];
            bool isValidPWM = false;
            
            // Lista de pines PWM válidos en ESP32
            const uint8_t validPWMPins[] = {0,1,2,3,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33};
            for (uint8_t j = 0; j < sizeof(validPWMPins); j++) {
                if (pin == validPWMPins[j]) {
                    isValidPWM = true;
                    break;
                }
            }
            
            if (!isValidPWM) {
                Serial.println(("[CONFIG ERROR] Pin servo " + std::to_string(i+1) + " (GPIO " + std::to_string(pin) + ") no es válido para PWM").c_str());
                return false;
            }
        }
        
        return true;
    }
    
    static bool validateServoConfiguration() {
        // Verificar que los tiempos de riego están dentro de límites seguros
        for (uint8_t i = 0; i < HardwarePins::Servos::COUNT; i++) {
            uint32_t time = ServoConfig::Timing::ZONE_TIMES[i];
            if (time < ServoConfig::Timing::MIN_IRRIGATION_TIME || 
                time > ServoConfig::Timing::MAX_IRRIGATION_TIME) {
                Serial.println(("[CONFIG ERROR] Tiempo de riego zona " + std::to_string(i+1) + " fuera de rango: " + std::to_string(time) + "s").c_str());
                return false;
            }
        }
        
        // Verificar configuración PWM
        if (ServoConfig::PWM::FREQUENCY < 20 || ServoConfig::PWM::FREQUENCY > 100) {
            Serial.println(("[CONFIG ERROR] Frecuencia PWM inválida: " + std::to_string(ServoConfig::PWM::FREQUENCY) + "Hz").c_str());
            return false;
        }
        
        return true;
    }
    
    static bool validateNetworkConfiguration() {
        // Verificar configuración de red
        if (NetworkConfig::WebServer::PORT < 1 || NetworkConfig::WebServer::PORT > 65535) {
            Serial.println(("[CONFIG ERROR] Puerto servidor web inválido: " + std::to_string(NetworkConfig::WebServer::PORT)).c_str());
            return false;
        }
        
        if (NetworkConfig::WiFi::CONNECTION_TIMEOUT < 5000) {
            Serial.println(("[CONFIG ERROR] Timeout WiFi muy corto: " + std::to_string(NetworkConfig::WiFi::CONNECTION_TIMEOUT) + "ms").c_str());
            return false;
        }
        
        return true;
    }
    
    static bool validateSecurityConfiguration() {
        // Verificar configuración de seguridad
        if (SecurityConfig::EEPROM::SIZE < 256) {
            Serial.println(("[CONFIG ERROR] Tamaño EEPROM muy pequeño: " + std::to_string(SecurityConfig::EEPROM::SIZE) + " bytes").c_str());
            return false;
        }
        
        if (SystemSafety::MAX_CONSECUTIVE_ERRORS < 1) {
            Serial.println(("[CONFIG ERROR] Número de errores consecutivos inválido: " + std::to_string(SystemSafety::MAX_CONSECUTIVE_ERRORS)).c_str());
            return false;
        }
        
        return true;
    }
    
    static bool validateSensorConfiguration() {
        // Verificar configuración de sensores
        if (SensorConfig::Humidity::DEFAULT_THRESHOLD > 100) {
            Serial.println(("[CONFIG ERROR] Umbral de humedad inválido: " + std::to_string(SensorConfig::Humidity::DEFAULT_THRESHOLD) + "%").c_str());
            return false;
        }
        
        if (SensorConfig::Monitoring::CRITICAL_MEMORY_THRESHOLD < 1000) {
            Serial.println(("[CONFIG ERROR] Umbral de memoria crítica muy bajo: " + std::to_string(SensorConfig::Monitoring::CRITICAL_MEMORY_THRESHOLD) + " bytes").c_str());
            return false;
        }
        
        return true;
    }
};

// =============================================================================
// MACROS DE UTILIDAD PARA DEBUGGING
// =============================================================================

#include <stdio.h>

#define DEBUG_PRINT(x) do { printf("%s", x); } while(0)
#define DEBUG_PRINTLN(x) do { printf("%s\n", x); } while(0)
#define DEBUG_PRINTF(format, ...) do { printf(format, ##__VA_ARGS__); } while(0)
#define VERBOSE_PRINT(x) do { printf("[VERBOSE] %s", x); } while(0)
#define VERBOSE_PRINTLN(x) do { printf("[VERBOSE] %s\n", x); } while(0)

// Función de utilidad para repetir caracteres
inline std::string repeatChar(char c, int times) {
    return std::string(times, c);
}

#endif // __PROJECT_CONFIG_H__
