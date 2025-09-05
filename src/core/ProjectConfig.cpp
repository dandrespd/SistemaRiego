#include <Arduino.h>
#include "ProjectConfig.h"
#include <ArduinoJson.h>

// Implementación de métodos para SystemConfig

bool SystemConfig::loadFromJson(const String& json) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return false;
    }

    // Cargar valores si existen en JSON
    if (doc.containsKey("wifi_ssid")) wifi_ssid = doc["wifi_ssid"].as<String>();
    if (doc.containsKey("wifi_password")) wifi_password = doc["wifi_password"].as<String>();
    if (doc.containsKey("ota_port")) ota_port = doc["ota_port"];
    if (doc.containsKey("ota_password")) ota_password = doc["ota_password"].as<String>();
    if (doc.containsKey("pin_servo")) pin_servo = doc["pin_servo"];
    if (doc.containsKey("pin_sensor_humedad")) pin_sensor_humedad = doc["pin_sensor_humedad"];
    if (doc.containsKey("num_servos")) num_servos = doc["num_servos"];
    // Para arrays como servo_pins, manejar con loop
    if (doc.containsKey("servo_pins")) {
        JsonArray arr = doc["servo_pins"];
        for (int i = 0; i < num_servos && i < arr.size(); i++) {
            servo_pins[i] = arr[i];
        }
    }
    if (doc.containsKey("rtc_rst")) rtc_rst = doc["rtc_rst"];
    if (doc.containsKey("rtc_sclk")) rtc_sclk = doc["rtc_sclk"];
    if (doc.containsKey("rtc_io")) rtc_io = doc["rtc_io"];
    if (doc.containsKey("led")) led = doc["led"];
    if (doc.containsKey("intervalo_lectura")) intervalo_lectura = doc["intervalo_lectura"];
    if (doc.containsKey("humedad_umbral")) humedad_umbral = doc["humedad_umbral"];
    if (doc.containsKey("max_total_irrigation_time_minutes")) max_total_irrigation_time_minutes = doc["max_total_irrigation_time_minutes"];
    if (doc.containsKey("watchdog_timeout_ms")) watchdog_timeout_ms = doc["watchdog_timeout_ms"];
    if (doc.containsKey("max_consecutive_errors")) max_consecutive_errors = doc["max_consecutive_errors"];
    if (doc.containsKey("web_server_port")) web_server_port = doc["web_server_port"];
    if (doc.containsKey("wifi_connection_timeout_ms")) wifi_connection_timeout_ms = doc["wifi_connection_timeout_ms"];
    if (doc.containsKey("max_wifi_retry_attempts")) max_wifi_retry_attempts = doc["max_wifi_retry_attempts"];
    if (doc.containsKey("enable_serial_debugging")) enable_serial_debugging = doc["enable_serial_debugging"];
    if (doc.containsKey("enable_verbose_logging")) enable_verbose_logging = doc["enable_verbose_logging"];
    if (doc.containsKey("serial_baud_rate")) serial_baud_rate = doc["serial_baud_rate"];

    return true;
}

String SystemConfig::saveToJson() const {
    DynamicJsonDocument doc(1024);

    doc["wifi_ssid"] = wifi_ssid;
    doc["wifi_password"] = wifi_password;
    doc["ota_port"] = ota_port;
    doc["ota_password"] = ota_password;
    doc["pin_servo"] = pin_servo;
    doc["pin_sensor_humedad"] = pin_sensor_humedad;
    doc["num_servos"] = num_servos;
    JsonArray servoArray = doc.createNestedArray("servo_pins");
    for (int i = 0; i < num_servos; i++) {
        servoArray.add(servo_pins[i]);
    }
    doc["rtc_rst"] = rtc_rst;
    doc["rtc_sclk"] = rtc_sclk;
    doc["rtc_io"] = rtc_io;
    doc["led"] = led;
    doc["intervalo_lectura"] = intervalo_lectura;
    doc["humedad_umbral"] = humedad_umbral;
    doc["max_total_irrigation_time_minutes"] = max_total_irrigation_time_minutes;
    doc["watchdog_timeout_ms"] = watchdog_timeout_ms;
    doc["max_consecutive_errors"] = max_consecutive_errors;
    doc["web_server_port"] = web_server_port;
    doc["wifi_connection_timeout_ms"] = wifi_connection_timeout_ms;
    doc["max_wifi_retry_attempts"] = max_wifi_retry_attempts;
    doc["enable_serial_debugging"] = enable_serial_debugging;
    doc["enable_verbose_logging"] = enable_verbose_logging;
    doc["serial_baud_rate"] = serial_baud_rate;

    String json;
    serializeJson(doc, json);
    return json;
}

// Instancia global
SystemConfig globalConfig;
