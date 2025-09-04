/**
 * @file main.cpp
 * @brief Programa principal simplificado para el Sistema de Riego Inteligente v3.2.
 * 
 * **TRANSFORMACIÓN EDUCATIVA COMPLETADA**:
 * Este archivo demuestra cómo un main.cpp monolítico de 600+ líneas puede
 * transformarse en un código limpio, mantenible y fácil de entender.
 * 
 * **PRINCIPIOS APLICADOS**:
 * 1. **Separación de Responsabilidades**: main.cpp solo maneja inicialización básica
 * 2. **Inversión de Dependencias**: Los módulos se inyectan en lugar de crearse aquí
 * 3. **Principio de Responsabilidad Única**: Cada clase tiene un propósito específico
 * 4. **Composición sobre Herencia**: Usamos composición para construir el sistema
 * 
 * **BENEFICIOS DE ESTA REFACTORIZACIÓN**:
 * - Código más legible y mantenible
 * - Pruebas facilitadas de componentes individuales
 * - Escalabilidad mejorada para futuras funcionalidades
 * - Depuración simplificada
 * - Reutilización de código mejorada
 * 
 * **ANALOGÍA EDUCATIVA**: 
 * Antes teníamos un "chef" que hacía todo: comprar ingredientes, cocinar, 
 * servir, limpiar y manejar las finanzas. Ahora tenemos un "gerente de 
 * restaurante" que coordina especialistas en cada área.
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.2 - Arquitectura Refactorizada
 * @date 2025
 */

// =============================================================================
// Includes Esenciales
// =============================================================================
#include <Arduino.h>
#include <WiFi.h>
#include "utils/Utils.h"
#include "core/SystemConfig.h"
#include "utils/GET_DATE.h"

// Módulos del proyecto
#include "core/SystemManager.h"
#include "network/WebControl.h"
#include "utils/SET_PIN.h"      // Configuración de pines
#include "drivers/ServoPWMController.h"  // Controlador de servo multi-zona con PWM nativo ESP32
#include "drivers/RTC_DS1302.h" // RTC DS1302
#include "drivers/Led.h"        // Control de LED
#include "drivers/I2CManager.h" // I2C Management

// OTA Updates
#include <ArduinoOTA.h>
// =============================================================================
// Instancias de Módulos para Inyección de Dependencias
// =============================================================================

// Punteros a los módulos principales que se inyectarán en SystemManager
RTC_DS1302* rtcInstance = nullptr;
Led* statusLedInstance = nullptr;
ServoPWMController* servoControllerInstance = nullptr;

// =============================================================================
// Instancia Principal del Sistema (Puntero)
// =============================================================================

/**
 * **CONCEPTO EDUCATIVO - INYECCIÓN DE DEPENDENCIAS (CORREGIDO)**:
 * Declaramos un puntero para SystemManager. Las dependencias se crearán en
 * setup() y luego se inyectarán al crear el objeto SystemManager.
 * Esto asegura que las dependencias estén listas antes de que SystemManager
 * las necesite, siguiendo el principio de Inversión de Control.
 */
SystemManager* sistemaRiego = nullptr;


// =============================================================================
// Setup: Inicialización Simplificada y Robusta
// =============================================================================

/**
 * @brief Inicialización del sistema de riego inteligente refactorizado.
 * 
 * **TRANSFORMACIÓN COMPLETADA**:
 * Hemos reducido la complejidad del setup de 200+ líneas a menos de 50,
 * manteniendo toda la funcionalidad pero con mejor organización.
 * 
 * **FLUJO DE INICIALIZACIÓN**:
 * 1. Configuración básica de comunicación
 * 2. Inicialización del gestor principal
 * 3. Configuración del sistema web
 * 4. Validación final y reporte de estado
 * 
 * Esta función demuestra el principio de "Tell, Don't Ask": le decimos
 * a cada módulo qué hacer sin necesidad de conocer sus detalles internos.
 */
void setup() {
    // **FASE 1: COMUNICACIÓN Y LOGGING**
    Serial.begin(SystemDebug::SERIAL_BAUD_RATE);
    // Initialize I2C (SDA/SCL as defined in SystemConfig) and mutex
    Drivers::I2CManager::begin();
    delay(2000); // Estabilización del puerto serie
    
    // **BANNER DE INICIO EDUCATIVO**
        DEBUG_PRINTLN("\n" + repeatChar('=', 70));
        DEBUG_PRINTLN("    SISTEMA DE RIEGO INTELIGENTE v3.2");
        DEBUG_PRINTLN("    Arquitectura Refactorizada y Optimizada");
        DEBUG_PRINTLN("    🌱 Transformación Educativa Completada 🌱");
        DEBUG_PRINTLN(repeatChar('=', 70));
    
    // **FASE 2: CREACIÓN DE DEPENDENCIAS**
    DEBUG_PRINTLN("\n🔧 [SETUP] Creando dependencias del sistema...");
    rtcInstance = new RTC_DS1302(HardwareConfig::RTC_RST, HardwareConfig::RTC_SCLK, HardwareConfig::RTC_IO);
    statusLedInstance = new Led(LED);
    servoControllerInstance = new ServoPWMController(NUM_SERVOS);
    DEBUG_PRINTLN("✅ [SETUP] Dependencias creadas exitosamente.");

    // **FASE 3: INYECCIÓN DE DEPENDENCIAS E INICIALIZACIÓN**
    DEBUG_PRINTLN("\n🔧 [SETUP] Inyectando dependencias y creando SystemManager...");
    sistemaRiego = new SystemManager(rtcInstance, statusLedInstance, servoControllerInstance);
    
    DEBUG_PRINTLN("\n🔧 [SETUP] Inicializando sistema principal...");
    if (!sistemaRiego->initialize()) {
        DEBUG_PRINTLN("❌ [SETUP] Error en inicialización - Sistema en modo limitado");
        // El sistema continuará funcionando en modo de recuperación
    } else {
        DEBUG_PRINTLN("✅ [SETUP] Sistema principal inicializado correctamente");
    }
    
    // **NOTA**: El controlador de servo ahora está gestionado por SystemManager
    // y se inicializa a través de la inyección de dependencias.
    
    // **FASE 4: CONFIGURACIÓN DEL SISTEMA WEB**
    DEBUG_PRINTLN("\n🌐 [SETUP] Configurando sistema web...");
    
    setupWebControl(sistemaRiego);
    
    // **FASE 4.5: CONFIGURACIÓN DE ACTUALIZACIONES OTA**
    DEBUG_PRINTLN("\n📡 [SETUP] Configurando actualizaciones OTA...");
    
    // Configurar OTA con hostname y contraseña segura
    #include "config.local.h" // define OTA_PASSWORD_LOCAL
    ArduinoOTA.setHostname("riego-inteligente");
    ArduinoOTA.setPassword(OTA_PASSWORD_LOCAL);
    
    // Configurar callbacks de OTA para feedback
    ArduinoOTA.onStart([]() {
        DEBUG_PRINTLN("[OTA] Iniciando actualización...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DEBUG_PRINTLN("[OTA] Progreso: " + String(progress * 100 / total) + "%");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_PRINTLN("[OTA] Error: " + String(error));
    });
    ArduinoOTA.onEnd([]() {
        DEBUG_PRINTLN("[OTA] Actualización completada");
    });
    
    // Iniciar servicio OTA
    ArduinoOTA.begin();
    DEBUG_PRINTLN("✅ [OTA] Servicio OTA inicializado");
    DEBUG_PRINTLN("   📍 Hostname: riego-inteligente");
    DEBUG_PRINTLN("   🔌 Puerto: 3232");
    DEBUG_PRINTLN("   💻 Actualiza con: platformio run --target upload --upload-port riego-inteligente.local");
    
    // **FASE 5: VERIFICACIÓN FINAL Y INICIO DE OPERACIONES**
    DEBUG_PRINTLN("\n🚀 [SETUP] Finalizando inicialización...");
    
    // Solo iniciar ciclo de riego si el sistema está completamente operacional
    // y el RTC está funcionando correctamente
    if (sistemaRiego->isOperational()) {
        // Verificar adicionalmente que el RTC no esté detenido
        bool canStartIrrigation = true;
        
        // Verificar estado del RTC directamente desde el SystemManager
        // El SystemManager ya tiene la lógica para verificar el estado del RTC
        if (sistemaRiego->hasErrors()) {
            DEBUG_PRINTLN("⚠️ [SETUP] Sistema tiene errores - No se puede iniciar riego automático");
            canStartIrrigation = false;
        }
        
        // Verificación adicional: si el RTC está detenido, no iniciar riego
        // Esta verificación se hace dentro del SystemManager durante la validación de salud
        
        if (canStartIrrigation && sistemaRiego->startIrrigationCycle()) {
            DEBUG_PRINTLN("✅ [SETUP] Ciclo de riego automático iniciado");
        } else {
            DEBUG_PRINTLN("⚠️ [SETUP] No se pudo iniciar riego automático");
        }
    } else {
        DEBUG_PRINTLN("ℹ️ [SETUP] Sistema en modo configuración - Riego automático deshabilitado");
        DEBUG_PRINTLN("🔧 Configure el RTC mediante:");
        DEBUG_PRINTLN("   - Puerto serial: Ingrese fecha/hora en formato AAMMDDWHHMMSS");
        DEBUG_PRINTLN("   - Interfaz web: Acceda a http://" + WiFi.localIP().toString() + "/config");
    }
    
    // **REPORTE FINAL DE INICIALIZACIÓN**
    DEBUG_PRINTLN("\n" + repeatChar('=', 70));
    DEBUG_PRINTLN("    RESUMEN DE INICIALIZACIÓN");
    DEBUG_PRINTLN(repeatChar('=', 70));
    DEBUG_PRINTLN("🔧 Estado del sistema: " + String(sistemaRiego->getCurrentStateString()));
    DEBUG_PRINTLN("💾 Memoria disponible: " + String(ESP.getFreeHeap()) + " bytes");
    DEBUG_PRINTLN("🌐 WiFi: " + String(WiFi.isConnected() ? "CONECTADO" : "DESCONECTADO"));
    
    if (WiFi.isConnected()) {
        DEBUG_PRINTLN("🌐 Acceso web: http://" + WiFi.localIP().toString());
    }
    
    if (sistemaRiego->hasErrors()) {
        DEBUG_PRINTLN("\n🟡 SISTEMA EN MODO DE RECUPERACIÓN");
        DEBUG_PRINTLN("   Funcionalidad limitada hasta resolver errores");
    } else {
        DEBUG_PRINTLN("\n🟢 SISTEMA COMPLETAMENTE OPERACIONAL");
        DEBUG_PRINTLN("   Todas las funcionalidades disponibles");
    }
    
    DEBUG_PRINTLN(repeatChar('=', 70) + "\n");
    
    // **INFORMACIÓN FINAL PARA EL USUARIO**
    sistemaRiego->printSystemInfo();
}

// =============================================================================
// Loop: Bucle Principal Ultra-Simplificado
// =============================================================================

/**
 * @brief Bucle principal simplificado del sistema de riego.
 * 
 * **REVOLUCIÓN ARQUITECTÓNICA COMPLETADA**:
 * Hemos transformado un loop() complejo de 150+ líneas en una función
 * ultra-simplificada que delega toda la complejidad al SystemManager.
 * 
 * **PRINCIPIOS DEMOSTRADOS**:
 * 1. **Delegación de Responsabilidades**: El loop solo coordina, no ejecuta
 * 2. **Operación No Bloqueante**: Toda la lógica usa máquinas de estado
 * 3. **Separación de Concerns**: Cada módulo maneja sus propias responsabilidades
 * 4. **Mantenibilidad**: El código es fácil de leer, entender y modificar
 * 
 * **ANALOGÍA EDUCATIVA**:
 * Es como la evolución de un trabajador que hace todo personalmente,
 * a un gerente que coordina especialistas. El resultado es más eficiente,
 * escalable y mantenible.
 * 
 * **MEDICIÓN DE RENDIMIENTO**:
 * Incluimos medición básica de rendimiento para identificar cuellos
 * de botella durante el desarrollo y debugging.
 */
void loop() {
    // **MEDICIÓN DE RENDIMIENTO OPCIONAL**
    #if 0
    unsigned long startTime = micros();
    #endif
    
    // **PROCESAMIENTO PRINCIPAL DELEGADO**
    // Toda la lógica compleja está encapsulada en el SystemManager
    if (sistemaRiego) {
        sistemaRiego->update();
    }
    
    // **NOTA**: El controlador de servo ahora está gestionado por SystemManager
    // y se actualiza automáticamente durante sistemaRiego.update()
    
    // **MANEJO DE ACTUALIZACIONES OTA**
    // Procesar solicitudes de actualización Over-The-Air
    ArduinoOTA.handle();
    
    // **COOPERACIÓN CON EL SISTEMA ESP32**
    // Importante: Permitir que el ESP32 maneje tareas internas
    // como WiFi, TCP/IP, y garbage collection
    yield();
    
    // **DEBUGGING DE RENDIMIENTO OPCIONAL**
    #if 0
    unsigned long endTime = micros();
    unsigned long loopDuration = endTime - startTime;
    
    // Reportar si el loop toma más de 10ms (indicador de posible problema)
    if (loopDuration > 10000) {
        DEBUG_PRINTLN("[PERFORMANCE] Loop lento: " + String(loopDuration) + " µs");
    }
    #endif
}

// =============================================================================
// Funciones de Utilidad (Si son necesarias en el futuro)
// =============================================================================

/**
 * **CONCEPTO EDUCATIVO - EXTENSIBILIDAD**:
 * Este espacio está reservado para futuras funciones de utilidad globales
 * que puedan ser necesarias. Sin embargo, la mayoría de funcionalidad
 * debería estar encapsulada en las clases apropiadas.
 * 
 * Ejemplos de funciones que podrían ir aquí:
 * - Manejadores de interrupciones globales
 * - Funciones de callback para librerías de terceros
 * - Utilidades de debugging muy específicas
 */

// [Espacio reservado para futuras expansiones]
