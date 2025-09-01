#ifndef __NTP_TIME_SYNC_H__
#define __NTP_TIME_SYNC_H__

/**
 * @file NTPTimeSync.h
 * @brief Módulo para sincronización automática de tiempo con servidores NTP.
 * 
 * **PRINCIPIOS APLICADOS**:
 * - Modularidad: Funcionalidad encapsulada en una clase independiente
 * - Robustez: Múltiples mecanismos de fallback y recuperación de errores
 * - Facilidad de uso: Configuración automática sin intervención manual
 * - Actualizabilidad: Sincronización periódica y manejo de desconexiones
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Mejoras de Fiabilidad
 * @date 2025
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "SystemConfig.h"
#include "RTC_DS1302.h"

class NTPTimeSync {
private:
    RTC_DS1302* rtc;
    unsigned long lastSyncTime;
    bool syncInProgress;
    uint8_t syncAttempts;
    
    // Métodos privados
    bool syncWithNTP();
    void setSystemTime(struct tm timeInfo);
    bool validateTime(struct tm timeInfo);
    
public:
    NTPTimeSync(RTC_DS1302* rtcInstance);
    
    // Inicialización
    bool initialize();
    
    // Sincronización
    bool syncTime();
    bool isTimeSynced() const;
    
    // Estado
    bool needsSync() const;
    uint8_t getSyncAttempts() const;
    
    // Utilidades
    static String getFormattedTime();
    static String getFormattedDate();
};

#endif // __NTP_TIME_SYNC_H__
