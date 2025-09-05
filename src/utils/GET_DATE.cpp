/**
 * @file GET_DATE.cpp
 * @brief Implementación robusta de la clase GetDate compatible con IRTC v2.0.
 * 
 * **CONCEPTO EDUCATIVO - ADAPTACIÓN A INTERFACES MEJORADAS**:
 * Como profesor siempre enseño que cuando una interfaz evoluciona, todo el código
 * que la usa debe adaptarse. Este archivo demuestra cómo migrar de una interfaz
 * simple a una interfaz robusta con manejo de errores.
 * 
 * **MEJORAS IMPLEMENTADAS**:
 * 1. **Error Handling**: Verificación de errores en todas las operaciones
 * 2. **Null Safety**: Validación de punteros antes de uso
 * 3. **Robust Formatting**: Formateo seguro con validación de datos
 * 4. **Performance Optimization**: Reducción de llamadas repetitivas al RTC
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0 - Compatible con IRTC v2.0
 * @date 2025
 */

#include "GET_DATE.h"
#include "SystemConfig.h"  // Para DEBUG_PRINTLN y configuración
#include <Arduino.h>

// =============================================================================
// Datos Estáticos de la Clase
// =============================================================================

// **ARRAY DE DÍAS**: Nombres en español para mejor UX
const char* GetDate::WeekDays[] = {
    "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Domingo"
};

// **ARRAY DE MESES**: Útil para formateo extendido
const char* GetDate::MonthNames[] = {
    "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio",
    "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"
};

// =============================================================================
// Constructor y Destructor
// =============================================================================

/**
 * @brief Constructor con inyección de dependencias mejorada.
 * 
 * **CONCEPTO EDUCATIVO**: La inyección de dependencias permite que esta clase
 * funcione con cualquier implementación de RTC sin modificar su código.
 * Es como tener un adaptador universal que funciona con cualquier enchufe.
 */
GetDate::GetDate(IRTC* rtc) 
    : rtc(rtc)
    , lastReadTime(0)
    , cachedDateTime()
    , isDataCached(false) {
    
    if (!rtc) {
        DEBUG_PRINTLN("[GET_DATE ERROR] RTC pointer is null in constructor");
    } else {
        DEBUG_PRINTLN("[GET_DATE] Constructor initialized successfully");
    }
}

/**
 * @brief Destructor que asegura limpieza de recursos.
 */
GetDate::~GetDate() {
    DEBUG_PRINTLN("[GET_DATE] Destructor executed");
    // No necesitamos liberar el RTC ya que no somos dueños del objeto
}

// =============================================================================
// Métodos Públicos Principales
// =============================================================================

/**
 * @brief Inicializa el RTC con verificación de errores.
 * 
 * **MEJORA SOBRE VERSIÓN ANTERIOR**: Ahora verificamos el resultado de
 * la inicialización y proporcionamos feedback claro sobre el estado.
 */
bool GetDate::init() {
    if (!rtc) {
        DEBUG_PRINTLN("[GET_DATE ERROR] No RTC available for initialization");
        return false;
    }
    
    DEBUG_PRINTLN("[GET_DATE] Initializing RTC...");
    
    if (rtc->init()) {
        DEBUG_PRINTLN("[GET_DATE] RTC initialized successfully");
        
        // **VERIFICACIÓN POST-INICIALIZACIÓN**: Comprobar que podemos leer datos
        DateTime testRead;
        if (rtc->getDateTime(&testRead)) {
            DEBUG_PRINTLN("[GET_DATE] RTC communication verified");
            return true;
        } else {
            DEBUG_PRINTLN("[GET_DATE WARNING] RTC initialized but communication failed");
            return false;
        }
    } else {
        DEBUG_PRINTLN("[GET_DATE ERROR] RTC initialization failed");
        return false;
    }
}

/**
 * @brief Verifica si el RTC está detenido con logging detallado.
 */
bool GetDate::isRtcHalted() {
    if (!rtc) {
        DEBUG_PRINTLN("[GET_DATE ERROR] No RTC available to check halted status");
        return true; // Asumir "detenido" si no hay RTC
    }
    
    bool halted = rtc->isHalted();
    
    if (halted) {
        DEBUG_PRINTLN("[GET_DATE] RTC is halted - requires configuration");
    } else {
        VERBOSE_PRINTLN("[GET_DATE] RTC is running normally");
    }
    
    return halted;
}

/**
 * @brief Imprime fecha/hora actual con optimización de rendimiento.
 * 
 * **CONCEPTO DE OPTIMIZACIÓN**: En lugar de llamar al RTC cada vez,
 * implementamos un cache inteligente que solo lee cuando es necesario.
 * Esto reduce la carga en el bus de comunicación y mejora el rendimiento.
 */
void GetDate::printDate() {
    if (!rtc) {
        Serial.println("[GET_DATE] RTC not available");
        return;
    }
    
    DateTime current;
    if (!getCurrentDateTime(current)) {
        Serial.println("[GET_DATE] Error reading current date/time");
        return;
    }
    
    // **ANTI-SPAM OPTIMIZATION**: Solo imprimir si ha cambiado el segundo
    static uint8_t lastPrintedSecond = 255; // Valor imposible para forzar primera impresión
    
    if (current.second != lastPrintedSecond) {
        lastPrintedSecond = current.second;
        
        // **FORMATEO MEJORADO**: Usar nuestro formateo personalizado
        String formattedDate = formatDate(current);
        Serial.println(formattedDate);
    }
}

/**
 * @brief Formatea fecha/hora con validación robusta.
 * 
 * **CONCEPTO EDUCATIVO**: El formateo seguro verifica los datos antes de usarlos.
 * Esto previene salidas incorrectas que podrían confundir al usuario.
 */
String GetDate::formatDate(const DateTime& dateTime) {
    // **VALIDACIÓN DE ENTRADA**: Verificar que la fecha sea válida
    if (!dateTime.isValid()) {
        return "[FECHA INVÁLIDA]";
    }
    
    // **FORMATEO SEGURO**: Usar buffer con tamaño adecuado
    char buffer[80]; // Buffer amplio para formateo completo
    
    // **VALIDACIÓN DE ÍNDICES**: Asegurar que los índices estén en rango
    const char* dayName = "Día Desconocido";
    if (dateTime.dayOfWeek >= 1 && dateTime.dayOfWeek <= 7) {
        dayName = WeekDays[dateTime.dayOfWeek - 1];
    }
    
    // **FORMATEO DESCRIPTIVO**: Formato legible para humanos
    int result = snprintf(buffer, sizeof(buffer), 
                         "Fecha: %02d/%02d/20%02d (%s) - Hora: %02d:%02d:%02d",
                         dateTime.day, dateTime.month, dateTime.year,
                         dayName,
                         dateTime.hour, dateTime.minute, dateTime.second);
    
    // **VERIFICACIÓN DE BUFFER**: Detectar truncamiento
    if (result >= sizeof(buffer)) {
        DEBUG_PRINTLN("[GET_DATE WARNING] Format buffer truncated");
    }
    
    return String(buffer);
}

/**
 * @brief Obtiene fecha/hora actual con cache inteligente.
 * 
 * **CONCEPTO DE CACHING**: Para mejorar rendimiento, cacheamos la lectura
 * del RTC por un breve período. Esto reduce la carga en el bus de comunicación.
 */
bool GetDate::getCurrentDateTime(DateTime& dateTime) {
    if (!rtc) {
        return false;
    }
    
    unsigned long currentTime = millis();
    
    // **CACHE STRATEGY**: Usar cache si la lectura es reciente (menos de 500ms)
    const unsigned long CACHE_VALIDITY_MS = 500;
    
    if (isDataCached && (currentTime - lastReadTime) < CACHE_VALIDITY_MS) {
        dateTime = cachedDateTime;
        VERBOSE_PRINTLN("[GET_DATE] Using cached date/time");
        return true;
    }
    
    // **FRESH READ**: Leer del RTC y actualizar cache
    if (rtc->getDateTime(&dateTime)) {
        // **UPDATE CACHE**: Actualizar cache con nuevos datos
        cachedDateTime = dateTime;
        lastReadTime = currentTime;
        isDataCached = true;
        
        VERBOSE_PRINTLN("[GET_DATE] Fresh date/time read from RTC");
        return true;
    } else {
        DEBUG_PRINTLN("[GET_DATE ERROR] Failed to read date/time from RTC");
        
        // **FALLBACK STRATEGY**: Si hay datos en cache, usarlos como fallback
        if (isDataCached) {
            dateTime = cachedDateTime;
            DEBUG_PRINTLN("[GET_DATE] Using stale cached data as fallback");
            return true;
        }
        
        return false;
    }
}

/**
 * @brief Formatea solo la fecha sin la hora.
 */
String GetDate::formatDateOnly(const DateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "[FECHA INVÁLIDA]";
    }
    
    const char* dayName = "Día Desconocido";
    if (dateTime.dayOfWeek >= 1 && dateTime.dayOfWeek <= 7) {
        dayName = WeekDays[dateTime.dayOfWeek - 1];
    }
    
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%02d/%02d/20%02d (%s)",
             dateTime.day, dateTime.month, dateTime.year, dayName);
    
    return String(buffer);
}

/**
 * @brief Formatea solo la hora sin la fecha.
 */
String GetDate::formatTimeOnly(const DateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "[HORA INVÁLIDA]";
    }
    
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             dateTime.hour, dateTime.minute, dateTime.second);
    
    return String(buffer);
}

/**
 * @brief Formatea fecha/hora en formato extendido con nombres de mes.
 */
String GetDate::formatDateExtended(const DateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "[FECHA INVÁLIDA]";
    }
    
    const char* dayName = "Día Desconocido";
    if (dateTime.dayOfWeek >= 1 && dateTime.dayOfWeek <= 7) {
        dayName = WeekDays[dateTime.dayOfWeek - 1];
    }
    
    const char* monthName = "Mes Desconocido";
    if (dateTime.month >= 1 && dateTime.month <= 12) {
        monthName = MonthNames[dateTime.month - 1];
    }
    
    char buffer[100];
    snprintf(buffer, sizeof(buffer), 
             "%s, %d de %s de 20%02d - %02d:%02d:%02d",
             dayName, dateTime.day, monthName, dateTime.year,
             dateTime.hour, dateTime.minute, dateTime.second);
    
    return String(buffer);
}

/**
 * @brief Invalida el cache forzando una nueva lectura del RTC.
 * 
 * **USO**: Llamar cuando se sabe que el RTC ha sido modificado externamente.
 */
void GetDate::invalidateCache() {
    isDataCached = false;
    DEBUG_PRINTLN("[GET_DATE] Cache invalidated");
}

/**
 * @brief Obtiene información de diagnóstico del módulo.
 */
String GetDate::getDiagnosticInfo() {
    String info = "=== DIAGNÓSTICO GET_DATE ===\n";
    
    info += "RTC disponible: " + String(rtc ? "Sí" : "No") + "\n";
    
    if (rtc) {
        info += "RTC detenido: " + String(isRtcHalted() ? "Sí" : "No") + "\n";
        info += "Cache activo: " + String(isDataCached ? "Sí" : "No") + "\n";
        
        if (isDataCached) {
            unsigned long cacheAge = millis() - lastReadTime;
            info += "Edad del cache: " + String(cacheAge) + " ms\n";
            info += "Fecha cacheada: " + formatDate(cachedDateTime) + "\n";
        }
        
        DateTime current;
        if (getCurrentDateTime(current)) {
            info += "Fecha actual: " + formatDate(current) + "\n";
        } else {
            info += "Error leyendo fecha actual\n";
        }
    }
    
    info += "=============================\n";
    
    return info;
}
