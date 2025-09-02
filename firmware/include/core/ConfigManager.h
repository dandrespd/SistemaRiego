#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

/**
 * @file ConfigManager.h
 * @brief Gestor de configuración persistente para sistema de riego inteligente.
 * 
 * Implementa un sistema unificado de configuración que permite:
 * - Almacenamiento en archivo JSON en SPIFFS
 * - Acceso via interfaz web y modificaciones en tiempo real
 * - Validación de parámetros y valores por defecto
 * - Copias de seguridad automáticas
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0
 * @date 2025
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <stdint.h>
#include <WString.h>
#include <String.h> // Include String support

// Número de zonas del sistema (configurable pero con máximo de 5)
constexpr uint8_t MAX_ZONES = 5;

/**
 * @brief Estructura de configuración para cada zona de riego.
 */
struct ZoneConfig {
    uint8_t zoneNumber;           // Número de zona (1-5)
    bool enabled;                 // Zona habilitada
    uint32_t irrigationTimeSec;   // Tiempo de riego en segundos
    uint32_t intervalMin;         // Intervalo entre activaciones en minutos
    uint8_t servoOpenAngle;       // Ángulo de apertura del servo (0-180)
    uint32_t transitionTimeMs;    // Tiempo de transición entre zonas en ms
};

/**
 * @brief Estructura de configuración global del sistema.
 */
struct SystemConfig {
    // Configuración de red
    String wifiSSID;
    String wifiPassword;
    bool wifiAPMode;

    // Configuración de RTC
    bool rtcAutoSync;
    String ntpServer;

    // Configuración de válvula principal
    uint32_t mainValveTimeSec;

    // Configuración de sensores
    int humidityThreshold;
    int temperatureThreshold;

    // Configuración de seguridad
    uint32_t maxIrrigationTimeMin;
    uint32_t emergencyTimeoutMs;
    uint8_t maxRetryAttempts;

    // Configuración de logging
    uint8_t logLevel; // 0: ERROR, 1: WARN, 2: INFO, 3: DEBUG
    bool logToFile;
    uint32_t logFileSizeKB;

    // Configuración de zonas
    ZoneConfig zones[MAX_ZONES];
};

/**
 * @class ConfigManager
 * @brief Gestor singleton para la configuración persistente del sistema.
 */
class ConfigManager {
private:
    static ConfigManager* instance;
    SystemConfig config;
    const char* configPath = "/config/system_config.json";
    const char* backupPath = "/config/backup/system_config.bak";
    bool _firstBoot = true; // First boot flag

    ConfigManager(); // Constructor privado para singleton

    bool loadConfiguration();
    bool saveConfiguration();
    bool validateConfiguration() const;
    void setDefaultConfiguration();

public:
    ~ConfigManager();

    // Eliminar copia y asignación para singleton
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    bool isFirstBoot() const;
    void setFirstBoot(bool firstBoot);

    /**
     * @brief Obtiene la instancia singleton del ConfigManager.
     */
    static ConfigManager& getInstance();

    /**
     * @brief Inicializa el gestor de configuración.
     */
    bool initialize();

    /**
     * @brief Obtiene la configuración actual del sistema.
     */
    const SystemConfig& getConfig() const;

    /**
     * @brief Actualiza la configuración completa del sistema.
     */
    bool updateConfig(const SystemConfig& newConfig);

    /**
     * @brief Actualiza la configuración de una zona específica.
     */
    bool updateZoneConfig(uint8_t zoneIndex, const ZoneConfig& zoneConfig);

    /**
     * @brief Restaura la configuración por defecto.
     */
    bool resetToDefaults();

    /**
     * @brief Exporta la configuración actual como JSON string.
     */
    String exportConfig() const;

    /**
     * @brief Importa configuración desde JSON string.
     */
    bool importConfig(const String& jsonConfig);

    /**
     * @brief Verifica la integridad del archivo de configuración.
     */
    bool checkConfigIntegrity() const;

    /**
     * @brief Obtiene el hash MD5 de la configuración actual para verificación.
     */
    String getConfigHash() const;

    /**
     * @brief Crea una copia de seguridad de la configuración.
     */
    bool createBackup();

    /**
     * @brief Restaura la configuración desde una copia de seguridad.
     */
    bool restoreBackup();
};

#endif // __CONFIG_MANAGER_H__
