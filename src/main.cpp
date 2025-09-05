/**
 * @file main.cpp
 * @brief Programa principal simplificado para el Sistema de Riego Inteligente v3.2.
 * 
 * Se han revertido los cambios de gestión de memoria debido a problemas de compilación.
 * Se mantienen todas las demás optimizaciones (GPIO, PSRAM, etc).
 */

#include <Arduino.h>
#include <WiFi.h>
#include "utils/Utils.h"
#include "core/SystemConfig.h"
#include "core/SystemManager.h"
#include "network/WebControl.h"
#include "drivers/ServoPWMController.h"
#include "drivers/RTC_DS1302.h"
#include "drivers/Led.h"
#include "drivers/I2CManager.h"
#include <ArduinoOTA.h>

RTC_DS1302* rtcInstance = nullptr;
Led* statusLedInstance = nullptr;
ServoPWMController* servoControllerInstance = nullptr;
SystemManager* sistemaRiego = nullptr;

void setup() {
    Serial.begin(SystemDebug::SERIAL_BAUD_RATE);
    Drivers::I2CManager::begin();
    delay(2000);
    
    DEBUG_PRINTLN("\n" + repeatChar('=', 70));
    DEBUG_PRINTLN("    SISTEMA DE RIEGO INTELIGENTE v3.3 - Optimizado");
    DEBUG_PRINTLN("    Conflictos GPIO resueltos - PSRAM habilitado");
    DEBUG_PRINTLN(repeatChar('=', 70));
    
    DEBUG_PRINTLN("\n🔧 [SETUP] Creando dependencias del sistema...");
    rtcInstance = new RTC_DS1302(HardwareConfig::RTC_RST, HardwareConfig::RTC_SCLK, HardwareConfig::RTC_IO);
    statusLedInstance = new Led(LED);
    servoControllerInstance = new ServoPWMController(NUM_SERVOS);
    DEBUG_PRINTLN("✅ [SETUP] Dependencias creadas exitosamente.");

    DEBUG_PRINTLN("\n🔧 [SETUP] Creando SystemManager...");
    sistemaRiego = new SystemManager(rtcInstance, statusLedInstance, servoControllerInstance);
    
    DEBUG_PRINTLN("\n🔧 [SETUP] Inicializando sistema principal...");
    if (!sistemaRiego->initialize()) {
        DEBUG_PRINTLN("❌ [SETUP] Error en inicialización - Sistema en modo limitado");
    } else {
        DEBUG_PRINTLN("✅ [SETUP] Sistema principal inicializado correctamente");
    }
    
    DEBUG_PRINTLN("\n🌐 [SETUP] Configurando sistema web...");
    setupWebControl(sistemaRiego);
    
    DEBUG_PRINTLN("\n📡 [SETUP] Configurando actualizaciones OTA...");
    #include "config.local.h"
    ArduinoOTA.setHostname("riego-inteligente");
    ArduinoOTA.setPassword(OTA_PASSWORD_LOCAL);
    
    ArduinoOTA.onStart([]() { DEBUG_PRINTLN("[OTA] Iniciando actualización..."); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DEBUG_PRINTLN("[OTA] Progreso: " + String(progress * 100 / total) + "%");
    });
    ArduinoOTA.onError([](ota_error_t error) { DEBUG_PRINTLN("[OTA] Error: " + String(error)); });
    ArduinoOTA.onEnd([]() { DEBUG_PRINTLN("[OTA] Actualización completada"); });
    
    ArduinoOTA.begin();
    DEBUG_PRINTLN("✅ [OTA] OTA inicializado");
}

void loop() {
    if (sistemaRiego) sistemaRiego->update();
    ArduinoOTA.handle();
    yield();
}
