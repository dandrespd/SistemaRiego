#pragma once

/**
 * Configuración centralizada del proyecto SistemaRiego
 * Contiene parámetros globales y configuraciones del sistema
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include <Arduino.h>

struct SystemConfig {
    // Configuración de red
    String wifi_ssid = "RIEGO_INTELIGENTE";
    String wifi_password = "riego2025secure";

    // Configuración OTA
    int ota_port = 3232;
    String ota_password = "riego2025secure";

    // Pines de hardware
    int pin_servo = 26;
    int pin_sensor_humedad = 35;

    // Pines para múltiples servos
    int num_servos = 4;
    int servo_pins[4] = {26, 27, 32, 33};

    // Pines RTC
    int rtc_rst = 25;
    int rtc_sclk = 14;
    int rtc_io = 13;

    // Pin LED
    int led = 2;

    // Parámetros del sistema
    int intervalo_lectura = 5000;  // ms
    float humedad_umbral = 40.0;   // %

    // Configuración de seguridad
    int max_total_irrigation_time_minutes = 120;   // 2 horas máx
    int watchdog_timeout_ms = 5000;                // 5 segundos
    int max_consecutive_errors = 5;                // 5 errores consecutivos

    // Configuración de red
    int web_server_port = 80;                     // Puerto HTTP
    int wifi_connection_timeout_ms = 10000;       // 10 segundos timeout
    int max_wifi_retry_attempts = 3;              // 3 reintentos

    // Configuración de debug
    bool enable_serial_debugging = true;
    bool enable_verbose_logging = false;
    int serial_baud_rate = 115200;

    // Métodos para load/save (implementar en ConfigManager)
    bool loadFromJson(const String& json);
    String saveToJson() const;
};

extern SystemConfig globalConfig;

#endif // PROJECT_CONFIG_H
