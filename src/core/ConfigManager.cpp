#include <Arduino.h>
#include <ConfigManager.h>
#include <ProjectConfig.h>
#include <Logger.h>
#include <ArduinoJson.h>

ConfigManager* ConfigManager::instance = nullptr;

const char* configPath = "/config.json";  // Ruta para el archivo de configuración

ConfigManager::ConfigManager() : _firstBoot(false) {}

ConfigManager* ConfigManager::getInstance() {
    if (instance == nullptr) {
        instance = new ConfigManager();
    }
    return instance;
}

bool ConfigManager::initialize() {
    if (!LittleFS.begin()) {
        LOG_ERROR("[CONFIG] Failed to mount LittleFS");
        return false;
    }

    _firstBoot = !LittleFS.exists(configPath);

    if (_firstBoot) {
        LOG_INFO("[CONFIG] First boot detected");
        setDefaultConfiguration();
        if (!saveConfiguration()) {
            LOG_ERROR("[CONFIG] Failed to save initial configuration");
            return false;
        }
        LOG_INFO("[CONFIG] Initial configuration saved");
    } else {
        if (!loadConfiguration()) {
            LOG_ERROR("[CONFIG] Failed to load configuration");
            return false;
        }
        LOG_INFO("[CONFIG] Configuration loaded");
    }
    return true;
}

void ConfigManager::setDefaultConfiguration() {
    // Establecer valores predeterminados en globalConfig (ya definidos en la struct)
    LOG_INFO("[CONFIG] Setting default configuration");
    
    // Los valores por defecto ya están establecidos en la struct SystemConfig
    // No necesitamos hacer nada adicional aquí
}

bool ConfigManager::setRTCTime(const String& datetime) {
    // Guardar configuración de fecha/hora del RTC
    File file = LittleFS.open("/rtc_config.json", "w");
    if (!file) {
        LOG_ERROR("[CONFIG] Failed to open RTC config file for writing");
        return false;
    }
    
    DynamicJsonDocument doc(256);
    doc["last_set_datetime"] = datetime;
    doc["timestamp"] = millis();
    
    if (serializeJson(doc, file) == 0) {
        LOG_ERROR("[CONFIG] Failed to write RTC config");
        file.close();
        return false;
    }
    
    file.close();
    LOG_INFO("[CONFIG] RTC configuration saved");
    return true;
}

String ConfigManager::getLastRTCTime() {
    File file = LittleFS.open("/rtc_config.json", "r");
    if (!file) {
        return "";
    }
    
    String json = file.readString();
    file.close();
    
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        return "";
    }
    
    return doc["last_set_datetime"].as<String>();
}

bool ConfigManager::updateConfigValue(const String& key, const String& value) {
    // Actualizar valor específico en la configuración
    if (key == "wifi_ssid") {
        globalConfig.wifi_ssid = value;
    } else if (key == "wifi_password") {
        globalConfig.wifi_password = value;
    } else if (key == "ota_password") {
        globalConfig.ota_password = value;
    } else if (key == "humedad_umbral") {
        globalConfig.humedad_umbral = value.toFloat();
    } else if (key == "intervalo_lectura") {
        globalConfig.intervalo_lectura = value.toInt();
    } else if (key == "watchdog_timeout_ms") {
        globalConfig.watchdog_timeout_ms = value.toInt();
    } else if (key == "max_consecutive_errors") {
        globalConfig.max_consecutive_errors = value.toInt();
    } else if (key == "web_server_port") {
        globalConfig.web_server_port = value.toInt();
    } else if (key == "enable_serial_debugging") {
        globalConfig.enable_serial_debugging = value.equalsIgnoreCase("true");
    } else if (key == "enable_verbose_logging") {
        globalConfig.enable_verbose_logging = value.equalsIgnoreCase("true");
    } else {
        LOG_WARNING("[CONFIG] Unknown configuration key: " + key);
        return false;
    }
    
    return saveConfiguration();
}

String ConfigManager::getConfigValue(const String& key) {
    if (key == "wifi_ssid") {
        return globalConfig.wifi_ssid;
    } else if (key == "wifi_password") {
        return globalConfig.wifi_password;
    } else if (key == "ota_password") {
        return globalConfig.ota_password;
    } else if (key == "humedad_umbral") {
        return String(globalConfig.humedad_umbral);
    } else if (key == "intervalo_lectura") {
        return String(globalConfig.intervalo_lectura);
    } else if (key == "watchdog_timeout_ms") {
        return String(globalConfig.watchdog_timeout_ms);
    } else if (key == "max_consecutive_errors") {
        return String(globalConfig.max_consecutive_errors);
    } else if (key == "web_server_port") {
        return String(globalConfig.web_server_port);
    } else if (key == "enable_serial_debugging") {
        return globalConfig.enable_serial_debugging ? "true" : "false";
    } else if (key == "enable_verbose_logging") {
        return globalConfig.enable_verbose_logging ? "true" : "false";
    }
    
    return "";
}

bool ConfigManager::loadConfiguration() {
    File file = LittleFS.open(configPath, "r");
    if (!file) {
        LOG_ERROR("[CONFIG] Failed to open config file");
        return false;
    }

    String json = file.readString();
    file.close();

    if (!globalConfig.loadFromJson(json)) {
        LOG_ERROR("[CONFIG] Failed to parse config JSON");
        return false;
    }

    return true;
}

bool ConfigManager::saveConfiguration() {
    String json = globalConfig.saveToJson();

    File file = LittleFS.open(configPath, "w");
    if (!file) {
        LOG_ERROR("[CONFIG] Failed to open config file for writing");
        return false;
    }

    if (file.print(json) == 0) {
        LOG_ERROR("[CONFIG] Failed to write config file");
        file.close();
        return false;
    }

    file.close();
    LOG_INFO("[CONFIG] Configuration saved successfully");
    return true;
}

bool ConfigManager::isFirstBoot() const {
    return _firstBoot;
}

void ConfigManager::setFirstBoot(bool firstBoot) {
    _firstBoot = firstBoot;
    saveConfiguration(); // Persist the state
}
