/**
 * @file RTC_DS1302.cpp
 * @brief Implementación completa de la clase RTC_DS1302 que cumple la interfaz IRTC.
 * 
 * **CONCEPTO EDUCATIVO - IMPLEMENTATION OF INTERFACES**:
 * Como profesor siempre enseño que cuando una clase implementa una interfaz,
 * debe cumplir exactamente el "contrato" que esa interfaz define. Es como
 * seguir una receta de cocina: si la receta dice "hornear por 30 minutos",
 * no puedes cambiar arbitrariamente el tiempo y esperar el mismo resultado.
 * 
 * Esta implementación demuestra:
 * 1. **Adapter Pattern**: Adaptamos la biblioteca Ds1302 a nuestra interfaz estándar
 * 2. **Error Handling**: Manejo robusto de errores en operaciones de RTC
 * 3. **Data Translation**: Conversión entre diferentes formatos de fecha/hora
 * 4. **Contract Compliance**: Cumplimiento exacto de la interfaz IRTC
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0 - Implementación conforme a interfaz IRTC
 * @date 2025
 */

#include "RTC_DS1302.h"
#include "SystemConfig.h"  // Para DEBUG_PRINTLN y configuración
#include <Arduino.h>

// Variables estáticas para throttling de mensajes
static unsigned long lastHaltedMessageTime = 0;
static bool lastHaltedState = false;
static unsigned long lastErrorRecoveryMessageTime = 0;
static const unsigned long MESSAGE_THROTTLE_INTERVAL = 10000; // 10 segundos

// =============================================================================
// Constructor y Destructor
// =============================================================================

/**
 * @brief Constructor que inicializa la conexión con el chip DS1302.
 * 
 * **CONCEPTO EDUCATIVO**: El constructor establece la configuración de hardware
 * pero NO inicia la comunicación inmediatamente. Esto permite crear el objeto
 * incluso si el hardware no está listo, y diferir la inicialización hasta
 * que sea apropiada.
 * 
 * @param rst_pin Pin de reset del DS1302
 * @param sclk_pin Pin de clock serial del DS1302
 * @param io_pin Pin de datos bidireccional del DS1302
 */
RTC_DS1302::RTC_DS1302(uint8_t rst_pin, uint8_t sclk_pin, uint8_t io_pin)
    : rtc(rst_pin, sclk_pin, io_pin)
    , isInitialized(false)
    , lastError(RTCError::NONE) {
    
    DEBUG_PRINTLN("[RTC_DS1302] Constructor - Pines: RST=" + String(rst_pin) + 
                 ", SCLK=" + String(sclk_pin) + ", IO=" + String(io_pin));
}

/**
 * @brief Destructor que asegura limpieza de recursos.
 */
RTC_DS1302::~RTC_DS1302() {
    DEBUG_PRINTLN("[RTC_DS1302] Destructor ejecutado");
}

// =============================================================================
// Implementación de la Interfaz IRTC
// =============================================================================

/**
 * @brief Inicializa el módulo RTC DS1302.
 * 
 * **CONCEPTO EDUCATIVO**: La inicialización en sistemas embebidos debe ser
 * robusta y verificar que el hardware responde correctamente. Es como hacer
 * un "handshake" - verificar que ambas partes pueden comunicarse antes de
 * proceder con operaciones más complejas.
 */
bool RTC_DS1302::init() {
    DEBUG_PRINTLN("[RTC_DS1302] Inicializando módulo RTC...");
    
    try {
        // **STEP 1**: Inicializar la biblioteca subyacente
        rtc.init();
        
        // **STEP 2**: Verificar comunicación leyendo un registro
        Ds1302::DateTime testRead;
        rtc.getDateTime(&testRead);
        
        // **STEP 3**: Validar que los datos leídos son razonables
        if (testRead.year > 99 || testRead.month > 12 || testRead.day > 31) {
            DEBUG_PRINTLN("[RTC_DS1302 ERROR] Datos inválidos leídos durante inicialización");
            lastError = RTCError::COMMUNICATION_FAILED;
            return false;
        }
        
        // **STEP 4**: Marcar como inicializado exitosamente
        isInitialized = true;
        lastError = RTCError::NONE;
        
        DEBUG_PRINTLN("[RTC_DS1302] Inicialización exitosa");
        return true;
        
    } catch (...) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] Excepción durante inicialización");
        lastError = RTCError::INITIALIZATION_FAILED;
        isInitialized = false;
        return false;
    }
}

/**
 * @brief Obtiene la fecha y hora actual del RTC.
 * 
 * **CONCEPTO EDUCATIVO**: Esta función implementa el patrón "Adapter"
 * convirtiendo entre el formato específico del DS1302 y nuestro formato
 * estándar. Es como un traductor que convierte entre dos idiomas manteniendo
 * el significado exacto.
 */
bool RTC_DS1302::getDateTime(DateTime* dateTime) {
    if (!dateTime) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] Puntero DateTime nulo");
        lastError = RTCError::INVALID_PARAMETER;
        return false;
    }
    
    if (!isInitialized) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] RTC no inicializado");
        lastError = RTCError::NOT_INITIALIZED;
        return false;
    }
    
    try {
        // **LECTURA DEL HARDWARE**: Obtener datos del chip DS1302
        Ds1302::DateTime raw_dt;
        if (!rtc.getDateTime(&raw_dt)) {
            DEBUG_PRINTLN("[RTC_DS1302 ERROR] Fallo de comunicación al leer RTC");
            lastError = RTCError::COMMUNICATION_FAILED;
            
            // Intentar recuperación automática
            if (init()) {
                DEBUG_PRINTLN("[RTC_DS1302] Recuperación automática exitosa");
                return getDateTime(dateTime); // Reintentar lectura
            }
            return false;
        }
        
        // **DATA TRANSLATION**: Convertir formato Ds1302 a nuestro formato estándar
        dateTime->year = raw_dt.year;
        dateTime->month = raw_dt.month;
        dateTime->day = raw_dt.day;
        dateTime->dayOfWeek = raw_dt.dow;
        dateTime->hour = raw_dt.hour;
        dateTime->minute = raw_dt.minute;
        dateTime->second = raw_dt.second;
        
        // **VALIDATION**: Verificar que los datos son lógicamente válidos
        if (!dateTime->isValid()) {
            String errorMsg = "[RTC_DS1302 ERROR] Fecha/hora inválida leída: ";
            errorMsg += String(raw_dt.year) + "-" + String(raw_dt.month) + "-" + String(raw_dt.day);
            errorMsg += " " + String(raw_dt.hour) + ":" + String(raw_dt.minute) + ":" + String(raw_dt.second);
            DEBUG_PRINTLN(errorMsg);
            
            lastError = RTCError::INVALID_DATA;
            
            // Si el RTC está detenido, intentar reiniciarlo
            if (isHalted()) {
                DEBUG_PRINTLN("[RTC_DS1302] Intentando reiniciar RTC detenido...");
                if (start()) {
                    return getDateTime(dateTime); // Reintentar lectura
                }
            }
            return false;
        }
        
        lastError = RTCError::NONE;
        VERBOSE_PRINTLN("[RTC_DS1302] Fecha/hora leída: " + dateTime->toString());
        return true;
        
    } catch (...) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] Excepción durante lectura");
        lastError = RTCError::COMMUNICATION_FAILED;
        return false;
    }
}

/**
 * @brief Establece la fecha y hora en el RTC.
 * 
 * **CONCEPTO EDUCATIVO**: Escribir al RTC es una operación crítica que puede
 * afectar todo el sistema. Implementamos validación exhaustiva antes de escribir
 * porque una fecha/hora incorrecta puede causar comportamientos impredecibles.
 */
bool RTC_DS1302::setDateTime(const DateTime& dateTime) {
    if (!isInitialized) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] RTC no inicializado");
        lastError = RTCError::NOT_INITIALIZED;
        return false;
    }
    
    // **PRE-VALIDATION**: Verificar datos antes de escribir
    if (!validateDateTime(dateTime)) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] Fecha/hora inválida para escribir");
        lastError = RTCError::INVALID_PARAMETER;
        return false;
    }
    
    try {
        // **DATA TRANSLATION**: Convertir nuestro formato al formato Ds1302
        Ds1302::DateTime raw_dt;
        raw_dt.year = dateTime.year;
        raw_dt.month = dateTime.month;
        raw_dt.day = dateTime.day;
        raw_dt.dow = dateTime.dayOfWeek;
        raw_dt.hour = dateTime.hour;
        raw_dt.minute = dateTime.minute;
        raw_dt.second = dateTime.second;
        
        // **WRITE OPERATION**: Escribir al chip DS1302 con reintentos
        const uint8_t maxRetries = 2;
        uint8_t retryCount = 0;
        bool writeSuccess = false;
        
        while (retryCount < maxRetries && !writeSuccess) {
            if (rtc.setDateTime(&raw_dt)) {
                writeSuccess = true;
            } else {
                retryCount++;
                if (retryCount < maxRetries) {
                    DEBUG_PRINTLN("[RTC_DS1302 WARNING] Reintentando escritura...");
                    delay(50);
                }
            }
        }
        
        if (!writeSuccess) {
            DEBUG_PRINTLN("[RTC_DS1302 ERROR] Fallo al escribir en RTC después de " + String(maxRetries) + " intentos");
            lastError = RTCError::COMMUNICATION_FAILED;
            return false;
        }
        
        // **VERIFICATION**: Leer de vuelta para verificar que se escribió correctamente
        DateTime verification;
        if (getDateTime(&verification)) {
            // Permitir pequeñas diferencias en segundos debido al tiempo de operación
            bool writeSuccessful = 
                (verification.year == dateTime.year) &&
                (verification.month == dateTime.month) &&
                (verification.day == dateTime.day) &&
                (verification.hour == dateTime.hour) &&
                (verification.minute == dateTime.minute);
                // No verificamos segundos exactos debido a latencia de operación
                
            if (writeSuccessful) {
                DEBUG_PRINTLN("[RTC_DS1302] Fecha/hora escrita exitosamente: " + dateTime.toString());
                lastError = RTCError::NONE;
                return true;
            } else {
                DEBUG_PRINTLN("[RTC_DS1302 ERROR] Verificación de escritura falló");
                lastError = RTCError::WRITE_VERIFICATION_FAILED;
                return false;
            }
        } else {
            DEBUG_PRINTLN("[RTC_DS1302 ERROR] No se pudo verificar escritura");
            lastError = RTCError::COMMUNICATION_FAILED;
            return false;
        }
        
    } catch (...) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] Excepción durante escritura");
        lastError = RTCError::COMMUNICATION_FAILED;
        return false;
    }
}

/**
 * @brief Verifica si el RTC está detenido o no funciona.
 * 
 * **CONCEPTO EDUCATIVO**: Los RTCs pueden detenerse por varias razones:
 * batería agotada, primer uso, interferencia, etc. Este método proporciona
 * diagnóstico crucial para troubleshooting.
 */
bool RTC_DS1302::isHalted() {
    if (!isInitialized) {
        // Si no está inicializado, consideramos que está "detenido"
        return true;
    }
    
    try {
        bool halted = rtc.isHalted();
        unsigned long currentTime = millis();
        
        if (halted) {
            // Throttling de mensajes: solo mostrar cada 10 segundos o si el estado cambió
            if (currentTime - lastHaltedMessageTime >= MESSAGE_THROTTLE_INTERVAL || halted != lastHaltedState) {
                DEBUG_PRINTLN("[RTC_DS1302] RTC está detenido");
                lastHaltedMessageTime = currentTime;
                lastHaltedState = halted;
            }
            lastError = RTCError::CLOCK_HALTED;
        } else {
            // Solo mostrar mensaje cuando el RTC se recupera o cada 30 segundos para confirmar funcionamiento
            if (lastHaltedState || currentTime - lastHaltedMessageTime >= 30000) {
                VERBOSE_PRINTLN("[RTC_DS1302] RTC funcionando correctamente");
                lastHaltedMessageTime = currentTime;
            }
            lastError = RTCError::NONE;
            lastHaltedState = halted;
        }
        
        return halted;
        
    } catch (...) {
        // Throttling para errores de comunicación también
        unsigned long currentTime = millis();
        if (currentTime - lastErrorRecoveryMessageTime >= MESSAGE_THROTTLE_INTERVAL) {
            DEBUG_PRINTLN("[RTC_DS1302 ERROR] Excepción verificando estado halted");
            lastErrorRecoveryMessageTime = currentTime;
        }
        lastError = RTCError::COMMUNICATION_FAILED;
        return true; // Asumir detenido si no podemos verificar
    }
}

/**
 * @brief Inicia/reanuda el funcionamiento del RTC.
 * 
 * **CONCEPTO EDUCATIVO**: Muchos RTCs vienen "detenidos" de fábrica para
 * conservar batería. Este método los activa para comenzar a mantener tiempo.
 */
bool RTC_DS1302::start() {
    if (!isInitialized) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] No se puede iniciar RTC no inicializado");
        lastError = RTCError::NOT_INITIALIZED;
        return false;
    }
    
    try {
        // **IMPLEMENTATION NOTE**: La biblioteca Ds1302 no tiene método start() explícito
        // Pero escribir una fecha/hora válida generalmente inicia el reloj
        
        // Si ya está funcionando, no hay nada que hacer
        if (!isHalted()) {
            DEBUG_PRINTLN("[RTC_DS1302] RTC ya está funcionando");
            return true;
        }
        
        // **WORKAROUND**: Escribir fecha/hora actual para iniciar el reloj
        // Esto es una técnica común para RTCs que no tienen comando start explícito
        DateTime defaultTime(25, 1, 1, 1, 0, 0, 0); // 1 de enero de 2025, 00:00:00
        
        if (setDateTime(defaultTime)) {
            DEBUG_PRINTLN("[RTC_DS1302] RTC iniciado con fecha por defecto");
            lastError = RTCError::NONE;
            return true;
        } else {
            DEBUG_PRINTLN("[RTC_DS1302 ERROR] No se pudo iniciar RTC");
            return false;
        }
        
    } catch (...) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] Excepción durante inicio de RTC");
        lastError = RTCError::COMMUNICATION_FAILED;
        return false;
    }
}

/**
 * @brief Detiene el funcionamiento del RTC.
 * 
 * **CONCEPTO EDUCATIVO**: Detener el RTC puede ser útil para conservar batería
 * en aplicaciones de ultra-bajo consumo, pero debe usarse con cuidado.
 */
bool RTC_DS1302::stop() {
    if (!isInitialized) {
        DEBUG_PRINTLN("[RTC_DS1302 ERROR] No se puede detener RTC no inicializado");
        lastError = RTCError::NOT_INITIALIZED;
        return false;
    }
    
    // **IMPLEMENTATION NOTE**: La biblioteca Ds1302 no proporciona método stop()
    // Esta es una limitación de la biblioteca subyacente
    DEBUG_PRINTLN("[RTC_DS1302 WARNING] Método stop() no implementado en biblioteca Ds1302");
    lastError = RTCError::NOT_SUPPORTED;
    return false;
}

// =============================================================================
// Métodos Específicos del DS1302
// =============================================================================

/**
 * @brief Obtiene el último error que ocurrió en el RTC.
 */
RTC_DS1302::RTCError RTC_DS1302::getLastError() const {
    return lastError;
}

/**
 * @brief Convierte código de error a string descriptivo.
 */
String RTC_DS1302::errorToString(RTCError error) {
    switch (error) {
        case RTCError::NONE:
            return "Sin errores";
        case RTCError::NOT_INITIALIZED:
            return "RTC no inicializado";
        case RTCError::COMMUNICATION_FAILED:
            return "Fallo de comunicación";
        case RTCError::INVALID_PARAMETER:
            return "Parámetro inválido";
        case RTCError::INVALID_DATA:
            return "Datos inválidos";
        case RTCError::CLOCK_HALTED:
            return "Reloj detenido";
        case RTCError::WRITE_VERIFICATION_FAILED:
            return "Verificación de escritura falló";
        case RTCError::INITIALIZATION_FAILED:
            return "Inicialización falló";
        case RTCError::NOT_SUPPORTED:
            return "Operación no soportada";
        default:
            return "Error desconocido";
    }
}

/**
 * @brief Obtiene información de diagnóstico específica del DS1302.
 */
String RTC_DS1302::getDiagnosticInfo() {
    String info = "=== DIAGNÓSTICO RTC DS1302 ===\n";
    
    info += "Inicializado: " + String(isInitialized ? "Sí" : "No") + "\n";
    info += "Último error: " + errorToString(lastError) + "\n";
    
    if (isInitialized) {
        info += "Estado detenido: " + String(isHalted() ? "Sí" : "No") + "\n";
        
        DateTime current;
        if (getDateTime(&current)) {
            info += "Fecha/hora actual: " + current.toString() + "\n";
            info += "Fecha válida: " + String(current.isValid() ? "Sí" : "No") + "\n";
        } else {
            info += "Error leyendo fecha/hora actual\n";
        }
    } else {
        info += "RTC no disponible para diagnóstico\n";
    }
    
    info += "==============================\n";
    
    return info;
}

/**
 * @brief Realiza un test completo del RTC.
 */
bool RTC_DS1302::performSelfTest() {
    DEBUG_PRINTLN("[RTC_DS1302] Iniciando auto-test...");
    
    // Test 1: Verificar inicialización
    if (!isInitialized) {
        if (!init()) {
            DEBUG_PRINTLN("[RTC_DS1302] Auto-test FALLÓ: No se pudo inicializar");
            return false;
        }
    }
    
    // Test 2: Verificar lectura
    DateTime testRead;
    if (!getDateTime(&testRead)) {
        DEBUG_PRINTLN("[RTC_DS1302] Auto-test FALLÓ: No se pudo leer fecha/hora");
        return false;
    }
    
    // Test 3: Test de escritura/lectura (solo si no está detenido)
    if (!isHalted()) {
        DateTime originalTime = testRead;
        DateTime testTime(25, 6, 15, 1, 12, 34, 56); // Fecha de test específica
        
        if (setDateTime(testTime)) {
            DateTime verifyTime;
            if (getDateTime(&verifyTime)) {
                // Restaurar tiempo original
                setDateTime(originalTime);
                
                // Verificar que el test de escritura funcionó
                if (verifyTime.year == testTime.year && 
                    verifyTime.month == testTime.month &&
                    verifyTime.day == testTime.day) {
                    DEBUG_PRINTLN("[RTC_DS1302] Auto-test EXITOSO");
                    return true;
                } else {
                    DEBUG_PRINTLN("[RTC_DS1302] Auto-test FALLÓ: Escritura no verificada");
                    return false;
                }
            } else {
                DEBUG_PRINTLN("[RTC_DS1302] Auto-test FALLÓ: No se pudo verificar escritura");
                return false;
            }
        } else {
            DEBUG_PRINTLN("[RTC_DS1302] Auto-test FALLÓ: No se pudo escribir");
            return false;
        }
    } else {
        DEBUG_PRINTLN("[RTC_DS1302] Auto-test PARCIAL: RTC detenido, solo test de lectura");
        return true;
    }
}
