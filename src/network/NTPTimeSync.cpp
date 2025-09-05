/**
 * @file NTPTimeSync.cpp
 * @brief Implementación de sincronización NTP para el sistema de riego
 * 
 * Implementa la sincronización automática de tiempo con servidores NTP
 * y actualización del RTC DS1302 según la configuración en SystemConfig.h
 */

#include "NTPTimeSync.h"
#include "SystemConfig.h"
#include "Logger.h"
#include <WiFi.h>
#include <time.h>

NTPTimeSync::NTPTimeSync(RTC_DS1302* rtcInstance) 
    : rtc(rtcInstance), lastSyncTime(0), syncInProgress(false), syncAttempts(0) {}

bool NTPTimeSync::initialize() {
    // Configurar cliente NTP con parámetros de SystemConfig
    configTime(
        RTCConfig::TIMEZONE_OFFSET * 3600,  // Convertir offset a segundos
        RTCConfig::DAYLIGHT_SAVING ? 3600 : 0, // Ajuste horario de verano
        RTCConfig::NTP_SERVER
    );
    return true;
}

bool NTPTimeSync::syncTime() {
    if (syncInProgress) {
        LOG_WARNING("NTP sync already in progress");
        return false;
    }

    // Verificar conexión WiFi primero
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR("NTP sync failed - WiFi not connected");
        return false;
    }

    syncInProgress = true;
    syncAttempts++;
    bool success = false;
    uint8_t retryCount = 0;
    const uint8_t maxRetries = 3;
    unsigned long retryDelay = 1000; // 1 segundo inicial

    while (retryCount < maxRetries && !success) {
        LOG_INFO("NTP sync attempt " + String(retryCount + 1) + 
                " with " + String(RTCConfig::NTP_SERVER));
        
        struct tm timeInfo;
        if (getLocalTime(&timeInfo, NetworkConfig::NTP_TIMEOUT_MS)) {
            LOG_INFO("NTP sync successful");
            setSystemTime(timeInfo);
            lastSyncTime = millis();
            success = true;
        } else {
            LOG_ERROR("NTP sync attempt " + String(retryCount + 1) + " failed");
            if (retryCount < maxRetries - 1) {
                delay(retryDelay);
                retryDelay *= 2; // Backoff exponencial
            }
            retryCount++;
        }
    }

    if (!success) {
        LOG_ERROR("NTP sync failed after " + String(maxRetries) + " attempts");
    }

    syncInProgress = false;
    return success;
}

void NTPTimeSync::setSystemTime(struct tm timeInfo) {
    if (!validateTime(timeInfo)) {
        LOG_ERROR("Invalid time received from NTP");
        return;
    }

    // Aplicar offset de zona horaria
    timeInfo.tm_hour += RTCConfig::TIMEZONE_OFFSET;
    mktime(&timeInfo); // Normalizar la estructura tm

    // Convertir a DateTime del RTC
    DateTime rtcTime(
        timeInfo.tm_year - 100, // Ajuste para años desde 2000
        timeInfo.tm_mon + 1,    // tm_mon es 0-11
        timeInfo.tm_mday,
        timeInfo.tm_wday % 7,   // Asegurar 0-6 (Dom-Sab)
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec
    );

    if (rtc->setDateTime(rtcTime)) {
        LOG_INFO("RTC updated from NTP: " + rtcTime.toString() + 
                " (UTC" + (RTCConfig::TIMEZONE_OFFSET >= 0 ? "+" : "") + 
                String(RTCConfig::TIMEZONE_OFFSET) + ")");
    } else {
        LOG_ERROR("Failed to update RTC from NTP. Last error: " + 
                 rtc->errorToString(rtc->getLastError()));
    }
}

bool NTPTimeSync::validateTime(struct tm timeInfo) {
    // Validación básica de rangos
    return (timeInfo.tm_year >= 120) && // 2020 en adelante (tm_year = año - 1900)
           (timeInfo.tm_mon >= 0 && timeInfo.tm_mon <= 11) &&
           (timeInfo.tm_mday >= 1 && timeInfo.tm_mday <= 31) &&
           (timeInfo.tm_hour >= 0 && timeInfo.tm_hour <= 23) &&
           (timeInfo.tm_min >= 0 && timeInfo.tm_min <= 59) &&
           (timeInfo.tm_sec >= 0 && timeInfo.tm_sec <= 59);
}

bool NTPTimeSync::isTimeSynced() const {
    return lastSyncTime > 0;
}

bool NTPTimeSync::needsSync() const {
    return (millis() - lastSyncTime) > NetworkConfig::NTP_UPDATE_INTERVAL_MS;
}

uint8_t NTPTimeSync::getSyncAttempts() const {
    return syncAttempts;
}

String NTPTimeSync::getFormattedTime() {
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        char buffer[9];
        strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeInfo);
        return String(buffer);
    }
    return "00:00:00";
}

String NTPTimeSync::getFormattedDate() {
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeInfo);
        return String(buffer);
    }
    return "2000-01-01";
}
