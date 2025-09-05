#pragma once

/**
 * Configuración centralizada del proyecto SistemaRiego
 * Contiene parámetros globales y configuraciones del sistema
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

// Configuración de red
const char* WIFI_SSID = "RIEGO_INTELIGENTE";
const char* WIFI_PASSWORD = "riego2025secure";

// Configuración OTA
const int OTA_PORT = 3232;
const char* OTA_PASSWORD = "riego2025secure";

// Pines de hardware
const int PIN_SERVO = 26;
const int PIN_SENSOR_HUMEDAD = 35;

// Parámetros del sistema
const int INTERVALO_LECTURA = 5000;  // ms
const float HUMEDAD_UMBRAL = 40.0;   // %

// Configuración de seguridad
const int MAX_TOTAL_IRRIGATION_TIME_MINUTES = 120;   // 2 horas máx
const int WATCHDOG_TIMEOUT_MS = 5000;                // 5 segundos
const int MAX_CONSECUTIVE_ERRORS = 5;                // 5 errores consecutivos

// Configuración de red
const int WEB_SERVER_PORT = 80;                     // Puerto HTTP
const int WIFI_CONNECTION_TIMEOUT_MS = 10000;       // 10 segundos timeout
const int MAX_WIFI_RETRY_ATTEMPTS = 3;              // 3 reintentos

// Configuración de hardware
const int NUM_SERVOS = 4;                           // 4 servos
const int SERVO_PINS[] = {26, 27, 32, 33};          // Pines para servos
const int RTC_RST = 25;                             // RST RTC
const int RTC_SCLK = 14;                            // SCLK RTC
const int RTC_IO = 13;                              // IO RTC
const int LED = 2;                                  // Pin LED

// Configuración de debug
const bool ENABLE_SERIAL_DEBUGGING = true;
const bool ENABLE_VERBOSE_LOGGING = false;
const int SERIAL_BAUD_RATE = 115200;

#endif // PROJECT_CONFIG_H
