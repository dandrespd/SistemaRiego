/**
 * @file ConfigManager.cpp
 * @brief Implementación del gestor de configuración persistente.
 * 
 * Maneja la carga, guardado y validación de la configuración del sistema
 * en formato JSON en el sistema de archivos SPIFFS.
 */

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <memory>
#include "core/ConfigManager.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "core/SystemConfig.h"

// Instancia singleton
ConfigManager* ConfigManager::instance = nullptr;

ConfigManager::ConfigManager() {
    // Inicializar configuración con valores por defecto
    setDefaultConfiguration();
}

ConfigManager::~ConfigManager() {
    // Guardar configuración al destruir
    saveConfiguration();
}

ConfigManager& ConfigManager::getInstance() {
    if (instance == nullptr) {
        instance = new ConfigManager();
    }
    return *instance;
}

bool ConfigManager::initialize() {
    // Inicializar SPIFFS si no está ya inicializado
    if (!SPIFFS.begin(true)) {
        // LOG_ERROR requires Serial to be initialized, but we can't use it here yet
        // Fall back to simple serial print if logging not available
        Serial.println("[ConfigManager] Error inicializando SPIFFS");
        return false;
    }

    // Crear directorio de configuración si no existe
    if (!SPIFFS.exists("/config")) {
        SPIFFS.mkdir("/config");
    }

    // Crear directorio de backup si no existe
    if (!SPIFFS.exists("/config/backup")) {
        SPIFFS.mkdir("/config/backup");
    }

    // Intentar cargar configuración existente
    if (!loadConfiguration()) {
        LOG_INFO("[ConfigManager] Cargando configuración por defecto");
        setDefaultConfiguration();
        return saveConfiguration();
    }

    return true;
}

bool ConfigManager::loadConfiguration() {
    if (!SPIFFS.exists(configPath)) {
        LOG_ERROR("[ConfigManager] Archivo de configuración no encontrado");
        return false;
    }

    File configFile = SPIFFS.open(configPath, "r");
    if (!configFile) {
        LOG_ERROR("[ConfigManager] Error abriendo archivo de configuración");
        return false;
    }

    size_t size = configFile.size();
    if (size > 2048) {
        LOG_ERROR("[ConfigManager] Archivo de configuración demasiado grande");
        configFile.close();
        return false;
    }

    std::unique_ptr<char[]> buf(new char[size + 1]);
    configFile.readBytes(buf.get(), size);
    buf.get()[size] = '\0';
    configFile.close();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error) {
        LOG_ERROR("[ConfigManager] Error parseando JSON: " + String(error.c_str()));
        return false;
    }

    // Cargar configuración de red
    config.wifiSSID = doc["wifi"]["ssid"] | "RiegoInteligente";
    config.wifiPassword = doc["wifi"]["password"] | "password123";
    config.wifiAPMode = doc["wifi"]["ap_mode"] | false;

    // Cargar configuración de RTC
    config.rtcAutoSync = doc["rtc"]["auto_sync"] | true;
    config.ntpServer = doc["rtc"]["ntp_server"] | "pool.ntp.org";

    // Cargar configuración de válvula principal
    config.mainValveTimeSec = doc["main_valve"]["time_sec"] | 300;

    // Cargar configuración de sensores
    config.humidityThreshold = doc["sensors"]["humidity_threshold"] | 40;
    config.temperatureThreshold = doc["sensors"]["temperature_threshold"] | 30;

    // Cargar configuración de seguridad
    config.maxIrrigationTimeMin = doc["safety"]["max_irrigation_time_min"] | 180;
    config.emergencyTimeoutMs = doc["safety"]["emergency_timeout_ms"] | 1000;
    config.maxRetryAttempts = doc["safety"]["max_retry_attempts"] | 5;

    // Cargar configuración de logging
    config.logLevel = doc["logging"]["level"] | 2;
    config.logToFile = doc["logging"]["to_file"] | false;
    config.logFileSizeKB = doc["logging"]["file_size_kb"] | 1024;

    // Cargar configuración de zonas
    JsonArray zonesArray = doc["zones"];
    for (int i = 0; i < MAX_ZONES && i < zonesArray.size(); i++) {
        JsonObject zoneObj = zonesArray[i];
        config.zones[i].zoneNumber = zoneObj["number"] | (i + 1);
        config.zones[i].enabled = zoneObj["enabled"] | true;
        config.zones[i].irrigationTimeSec = zoneObj["irrigation_time_sec"] | 60;
        config.zones[i].intervalMin = zoneObj["interval_min"] | 60;
        config.zones[i].servoOpenAngle = zoneObj["servo_open_angle"] | 90;
        config.zones[i].transitionTimeMs = zoneObj["transition_time_ms"] | 1000;
    }

    LOG_INFO("[ConfigManager] Configuración cargada exitosamente");
    return true;
}


bool ConfigManager::saveConfiguration() {
    DynamicJsonDocument doc(2048);

    // Guardar configuración de red
    doc["wifi"]["ssid"] = config.wifiSSID;
    doc["wifi"]["password"] = config.wifiPassword;
    doc["wifi"]["ap_mode"] = config.wifiAPMode;

    // Guardar configuración de RTC
    doc["rtc"]["auto_sync"] = config.rtcAutoSync;
    doc["rtc"]["ntp_server"] = config.ntpServer;

    // Guardar configuración de válvula principal
    doc["main_valve"]["time_sec"] = config.mainValveTimeSec;

    // Guardar configuración de sensores
    doc["sensors"]["humidity_threshold"] = config.humidityThreshold;
    doc["sensors"]["temperature_threshold"] = config.temperatureThreshold;

    // Guardar configuración de seguridad
    doc["safety"]["max_irrigation_time_min"] = config.maxIrrigationTimeMin;
    doc["safety"]["emergency_timeout_ms"] = config.emergencyTimeoutMs;
    doc["safety"]["max_retry_attempts"] = config.maxRetryAttempts;

    // Guardar configuración de logging
    doc["logging"]["level"] = config.logLevel;
    doc["logging"]["to_file"] = config.logToFile;
    doc["logging"]["file_size_kb"] = config.logFileSizeKB;

    // Guardar configuración de zonas
    JsonArray zonesArray = doc.createNestedArray("zones");
    for (int i = 0; i < MAX_ZONES; i++) {
        JsonObject zoneObj = zonesArray.createNestedObject();
        zoneObj["number"] = config.zones[i].zoneNumber;
        zoneObj["enabled"] = config.zones[i].enabled;
        zoneObj["irrigation_time_sec"] = config.zones[i].irrigationTimeSec;
        zoneObj["interval_min"] = config.zones[i].intervalMin;
        zoneObj["servo_open_angle"] = config.zones[i].servoOpenAngle;
        zoneObj["transition_time_ms"] = config.zones[i].transitionTimeMs;
    }

    // Escritura atómica: escribir primero a un archivo temporal
    String tempPath = String(configPath) + ".tmp";
    File tempFile = SPIFFS.open(tempPath, "w");
    if (!tempFile) {
        LOG_ERROR("[ConfigManager] Error creando archivo temporal de configuración");
        return false;
    }

    serializeJson(doc, tempFile);
    tempFile.close();

    // Verificar que el archivo temporal se escribió correctamente
    File verifyFile = SPIFFS.open(tempPath, "r");
    if (!verifyFile) {
        LOG_ERROR("[ConfigManager] Error verificando archivo temporal");
        SPIFFS.remove(tempPath);
        return false;
    }

    // Verificar que el contenido no está vacío
    if (verifyFile.size() == 0) {
        LOG_ERROR("[ConfigManager] Archivo temporal vacío");
        verifyFile.close();
        SPIFFS.remove(tempPath);
        return false;
    }
    verifyFile.close();

    // Reemplazar el archivo original con el temporal (operación atómica)
    if (SPIFFS.exists(configPath)) {
        SPIFFS.remove(configPath);
    }

    if (!SPIFFS.rename(tempPath, configPath)) {
        LOG_ERROR("[ConfigManager] Error renombrando archivo temporal");
        SPIFFS.remove(tempPath);
        return false;
    }

    // Crear backup automático
    createBackup();

    LOG_INFO("[ConfigManager] Configuración guardada exitosamente (escritura atómica)");
    return true;
}

bool ConfigManager::validateConfiguration() const {
    // Validar valores mínimos y máximos
    if (config.mainValveTimeSec < 10 || config.mainValveTimeSec > 3600) {
        LOG_ERROR("[ConfigManager] Tiempo de válvula principal inválido");
        return false;
    }

    for (int i = 0; i < MAX_ZONES; i++) {
        if (config.zones[i].irrigationTimeSec < 5 || config.zones[i].irrigationTimeSec > 3600) {
            LOG_ERROR("[ConfigManager] Tiempo de riego inválido para zona " + String(i + 1));
            return false;
        }
        if (config.zones[i].servoOpenAngle > 180) {
            LOG_ERROR("[ConfigManager] Ángulo de servo inválido para zona " + String(i + 1));
            return false;
        }
    }

    return true;
}

void ConfigManager::setDefaultConfiguration() {
    // Configuración por defecto de red
    config.wifiSSID = "RiegoInteligente";
    config.wifiPassword = "password123";
    config.wifiAPMode = false;

    // Configuración por defecto de RTC
    config.rtcAutoSync = true;
    config.ntpServer = "pool.ntp.org";

    // Configuración por defecto de válvula principal
    config.mainValveTimeSec = 300;

    // Configuración por defecto de sensores
    config.humidityThreshold = 40;
    config.temperatureThreshold = 30;

    // Configuración por defecto de seguridad
    config.maxIrrigationTimeMin = 180;
    config.emergencyTimeoutMs = 1000;
    config.maxRetryAttempts = 5;

    // Configuración por defecto de logging
    config.logLevel = 2; // INFO
    config.logToFile = false;
    config.logFileSizeKB = 1024;

    // Configuración por defecto de zonas
    for (int i = 0; i < MAX_ZONES; i++) {
        config.zones[i].zoneNumber = i + 1;
        config.zones[i].enabled = true;
        config.zones[i].irrigationTimeSec = 60;
        config.zones[i].intervalMin = 60;
        config.zones[i].servoOpenAngle = 90;
        config.zones[i].transitionTimeMs = 1000;
    }
}

bool ConfigManager::createBackup() {
    if (!SPIFFS.exists(configPath)) {
        return false;
    }

    File sourceFile = SPIFFS.open(configPath, "r");
    File backupFile = SPIFFS.open(backupPath, "w");
    
    if (!sourceFile || !backupFile) {
        if (sourceFile) sourceFile.close();
        if (backupFile) backupFile.close();
        return false;
    }

    backupFile.write(sourceFile.read());
    sourceFile.close();
    backupFile.close();

    return true;
}

bool ConfigManager::restoreBackup() {
    if (!SPIFFS.exists(backupPath)) {
        return false;
    }

    File backupFile = SPIFFS.open(backupPath, "r");
    File configFile = SPIFFS.open(configPath, "w");
    
    if (!backupFile || !configFile) {
        if (backupFile) backupFile.close();
        if (configFile) configFile.close();
        return false;
    }

    configFile.write(backupFile.read());
    backupFile.close();
    configFile.close();

    return loadConfiguration();
}

const SystemConfig& ConfigManager::getConfig() const {
    return config;
}

bool ConfigManager::updateConfig(const SystemConfig& newConfig) {
    config = newConfig;
    if (!validateConfiguration()) {
        return false;
    }
    return saveConfiguration();
}

bool ConfigManager::updateZoneConfig(uint8_t zoneIndex, const ZoneConfig& zoneConfig) {
    if (zoneIndex >= MAX_ZONES) {
        return false;
    }
    config.zones[zoneIndex] = zoneConfig;
    if (!validateConfiguration()) {
        return false;
    }
    return saveConfiguration();
}

bool ConfigManager::resetToDefaults() {
    setDefaultConfiguration();
    return saveConfiguration();
}

String ConfigManager::exportConfig() const {
    DynamicJsonDocument doc(2048);
    
    doc["wifi"]["ssid"] = config.wifiSSID;
    doc["wifi"]["password"] = config.wifiPassword;
    doc["wifi"]["ap_mode"] = config.wifiAPMode;
    
    doc["rtc"]["auto_sync"] = config.rtcAutoSync;
    doc["rtc"]["ntp_server"] = config.ntpServer;
    
    doc["main_valve"]["time_sec"] = config.mainValveTimeSec;
    
    doc["sensors"]["humidity_threshold"] = config.humidityThreshold;
    doc["sensors"]["temperature_threshold"] = config.temperatureThreshold;
    
    doc["safety"]["max_irrigation_time_min"] = config.maxIrrigationTimeMin;
    doc["safety"]["emergency_timeout_ms"] = config.emergencyTimeoutMs;
    doc["safety"]["max_retry_attempts"] = config.maxRetryAttempts;
    
    doc["logging"]["level"] = config.logLevel;
    doc["logging"]["to_file"] = config.logToFile;
    doc["logging"]["file_size_kb"] = config.logFileSizeKB;
    
    JsonArray zonesArray = doc.createNestedArray("zones");
    for (int i = 0; i < MAX_ZONES; i++) {
        JsonObject zoneObj = zonesArray.createNestedObject();
        zoneObj["number"] = config.zones[i].zoneNumber;
        zoneObj["enabled"] = config.zones[i].enabled;
        zoneObj["irrigation_time_sec"] = config.zones[i].irrigationTimeSec;
        zoneObj["interval_min"] = config.zones[i].intervalMin;
        zoneObj["servo_open_angle"] = config.zones[i].servoOpenAngle;
        zoneObj["transition_time_ms"] = config.zones[i].transitionTimeMs;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool ConfigManager::importConfig(const String& jsonConfig) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonConfig);
    if (error) {
        return false;
    }
    
    // Actualizar configuración desde JSON
    config.wifiSSID = doc["wifi"]["ssid"] | config.wifiSSID;
    config.wifiPassword = doc["wifi"]["password"] | config.wifiPassword;
    config.wifiAPMode = doc["wifi"]["ap_mode"] | config.wifiAPMode;
    
    config.rtcAutoSync = doc["rtc"]["auto_sync"] | config.rtcAutoSync;
    config.ntpServer = doc["rtc"]["ntp_server"] | config.ntpServer;
    
    config.mainValveTimeSec = doc["main_valve"]["time_sec"] | config.mainValveTimeSec;
    
    config.humidityThreshold = doc["sensors"]["humidity_threshold"] | config.humidityThreshold;
    config.temperatureThreshold = doc["sensors"]["temperature_threshold"] | config.temperatureThreshold;
    
    config.maxIrrigationTimeMin = doc["safety"]["max_irrigation_time_min"] | config.maxIrrigationTimeMin;
    config.emergencyTimeoutMs = doc["safety"]["emergency_timeout_ms"] | config.emergencyTimeoutMs;
    config.maxRetryAttempts = doc["safety"]["max_retry_attempts"] | config.maxRetryAttempts;
    
    config.logLevel = doc["logging"]["level"] | config.logLevel;
    config.logToFile = doc["logging"]["to_file"] | config.logToFile;
    config.logFileSizeKB = doc["logging"]["file_size_kb"] | config.logFileSizeKB;
    
    JsonArray zonesArray = doc["zones"];
    for (int i = 0; i < MAX_ZONES && i < zonesArray.size(); i++) {
        JsonObject zoneObj = zonesArray[i];
        config.zones[i].zoneNumber = zoneObj["number"] | config.zones[i].zoneNumber;
        config.zones[i].enabled = zoneObj["enabled"] | config.zones[i].enabled;
        config.zones[i].irrigationTimeSec = zoneObj["irrigation_time_sec"] | config.zones[i].irrigationTimeSec;
        config.zones[i].intervalMin = zoneObj["interval_min"] | config.zones[i].intervalMin;
        config.zones[i].servoOpenAngle = zoneObj["servo_open_angle"] | config.zones[i].servoOpenAngle;
        config.zones[i].transitionTimeMs = zoneObj["transition_time_ms"] | config.zones[i].transitionTimeMs;
    }
    
    if (!validateConfiguration()) {
        return false;
    }
    return saveConfiguration();
}

bool ConfigManager::checkConfigIntegrity() const {
    // Verificar que todos los valores están dentro de rangos válidos
    return validateConfiguration();
}

String ConfigManager::getConfigHash() const {
    // Simple hash calculation for verification
    String configStr = exportConfig();
    uint16_t hash = 0;
    for (char c : configStr) {
        hash = (hash << 5) + hash + c;
    }
    return String(hash, HEX);
}
