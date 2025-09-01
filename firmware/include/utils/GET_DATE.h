/**
 * @file GET_DATE.h
 * @brief Clase avanzada para obtener y formatear fecha/hora del RTC con IRTC v2.0.
 * 
 * **CONCEPTO EDUCATIVO - EVOLUCIÓN DE INTERFACES**:
 * Como profesor siempre enseño que cuando las interfaces evolucionan, las clases
 * que las usan deben evolucionar también. Esta clase demuestra cómo migrar de
 * una interfaz simple a una interfaz robusta manteniendo la funcionalidad.
 * 
 * **MEJORAS IMPLEMENTADAS EN ESTA VERSIÓN**:
 * 1. **Compatibilidad IRTC v2.0**: Funciona con la nueva interfaz robusta
 * 2. **Cache Inteligente**: Reduce llamadas al RTC para mejor rendimiento
 * 3. **Múltiples Formatos**: Diferentes opciones de formateo para distintos usos
 * 4. **Error Handling**: Manejo robusto de errores en todas las operaciones
 * 5. **Null Safety**: Validación de punteros antes de su uso
 * 6. **Diagnósticos**: Capacidades de debugging y monitoreo integradas
 * 
 * **PATRÓN DE DISEÑO**: Esta clase implementa:
 * - **Facade Pattern**: Simplifica el acceso a funcionalidades complejas del RTC
 * - **Caching Strategy**: Optimiza rendimiento mediante cache inteligente
 * - **Template Method**: Define flujos de formateo reutilizables
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0 - Compatible con IRTC v2.0 y optimizaciones avanzadas
 * @date 2025
 */
#ifndef __GET_DATE_H__
#define __GET_DATE_H__

#include <Arduino.h>
#include "IRTC.h"          // Interfaz genérica de RTC v2.0

/**
 * @class GetDate
 * @brief Clase avanzada para obtener y formatear fecha/hora con cache inteligente.
 * 
 * **CONCEPTO EDUCATIVO - FACADE + CACHE PATTERN**:
 * Esta clase actúa como una "fachada" que simplifica el acceso al RTC mientras
 * proporciona optimizaciones de rendimiento y múltiples opciones de formateo.
 * Es como tener un asistente personal que no solo sabe la hora, sino que la
 * presenta de diferentes maneras según tus necesidades.
 * 
 * **CARACTERÍSTICAS AVANZADAS**:
 * - **Cache inteligente**: Evita lecturas innecesarias del RTC
 * - **Múltiples formatos**: Fecha sola, hora sola, formato extendido, etc.
 * - **Validación robusta**: Verifica datos antes de formatear
 * - **Manejo de errores**: Respuestas elegantes ante fallos
 * - **Diagnósticos**: Información detallada para debugging
 * 
 * **EJEMPLO DE USO AVANZADO**:
 * ```cpp
 * // Crear instancia con inyección de dependencias
 * GetDate fechaHora(&mi_rtc);
 * 
 * // Inicialización robusta
 * if (!fechaHora.init()) {
 *     Serial.println("Error inicializando módulo de fecha");
 *     return;
 * }
 * 
 * // Verificar estado del RTC
 * if (fechaHora.isRtcHalted()) {
 *     Serial.println("RTC requiere configuración");
 * }
 * 
 * // Imprimir fecha con formato automático
 * fechaHora.printDate();
 * 
 * // Obtener formatos específicos
 * DateTime ahora;
 * if (fechaHora.getCurrentDateTime(ahora)) {
 *     Serial.println("Fecha: " + fechaHora.formatDateOnly(ahora));
 *     Serial.println("Hora: " + fechaHora.formatTimeOnly(ahora));
 *     Serial.println("Completo: " + fechaHora.formatDateExtended(ahora));
 * }
 * 
 * // Diagnóstico del sistema
 * Serial.println(fechaHora.getDiagnosticInfo());
 * ```
 */
class GetDate {
private:
    // =========================================================================
    // Atributos Privados
    // =========================================================================
    
    IRTC* rtc;                          // Puntero a objeto RTC compatible con IRTC
    
    // **SISTEMA DE CACHE INTELIGENTE**
    unsigned long lastReadTime;         // Timestamp de última lectura del RTC
    DateTime cachedDateTime;            // Última fecha/hora leída y cacheada
    bool isDataCached;                  // Flag indicando si hay datos válidos en cache
    
    // **ARRAYS ESTÁTICOS PARA FORMATEO**
    static const char* WeekDays[];      // Nombres de días de la semana en español
    static const char* MonthNames[];    // Nombres de meses en español

public:
    // =========================================================================
    // Constructor y Destructor
    // =========================================================================
    
    /**
     * @brief Constructor con inyección de dependencias.
     * 
     * **CONCEPTO DE DEPENDENCY INJECTION**: Recibe una referencia al RTC
     * en lugar de crear su propia instancia. Esto permite flexibilidad
     * y facilita testing con mocks.
     * 
     * @param rtc Puntero a objeto que implementa la interfaz IRTC
     */
    explicit GetDate(IRTC* rtc);
    
    /**
     * @brief Destructor que asegura limpieza de recursos.
     */
    ~GetDate();
    
    // =========================================================================
    // Métodos Públicos Principales
    // =========================================================================
    
    /**
     * @brief Inicializa el RTC con verificación robusta.
     * 
     * **MEJORA SOBRE VERSIÓN ANTERIOR**: Ahora verifica el resultado de
     * la inicialización y proporciona feedback claro sobre errores.
     * 
     * @return true si inicialización fue exitosa, false en caso de error
     */
    bool init();
    
    /**
     * @brief Imprime fecha/hora actual al puerto serie con optimización.
     * 
     * **OPTIMIZACIÓN**: Usa cache inteligente y anti-spam para evitar
     * imprimir la misma fecha múltiples veces por segundo.
     */
    void printDate();
    
    /**
     * @brief Verifica si el RTC está detenido.
     * 
     * @return true si el RTC está detenido y requiere configuración
     */
    bool isRtcHalted();
    
    /**
     * @brief Obtiene fecha/hora actual con cache inteligente.
     * 
     * **CARACTERÍSTICA NUEVA**: Implementa cache para reducir llamadas al RTC
     * y mejorar rendimiento del sistema.
     * 
     * @param dateTime Referencia donde almacenar la fecha/hora obtenida
     * @return true si se obtuvo fecha/hora válida
     */
    bool getCurrentDateTime(DateTime& dateTime);
    
    // =========================================================================
    // Métodos de Formateo (Múltiples Opciones)
    // =========================================================================
    
    /**
     * @brief Formatea fecha/hora completa con validación robusta.
     * 
     * **FORMATO**: "Fecha: DD/MM/AAAA (Nombre_Día) - Hora: HH:MM:SS"
     * 
     * @param dateTime Estructura DateTime a formatear
     * @return String con fecha/hora formateada o mensaje de error
     */
    String formatDate(const DateTime& dateTime);
    
    /**
     * @brief Formatea solo la fecha sin la hora.
     * 
     * **FORMATO**: "DD/MM/AAAA (Nombre_Día)"
     * 
     * @param dateTime Estructura DateTime a formatear
     * @return String con fecha formateada
     */
    String formatDateOnly(const DateTime& dateTime);
    
    /**
     * @brief Formatea solo la hora sin la fecha.
     * 
     * **FORMATO**: "HH:MM:SS"
     * 
     * @param dateTime Estructura DateTime a formatear
     * @return String con hora formateada
     */
    String formatTimeOnly(const DateTime& dateTime);
    
    /**
     * @brief Formatea fecha/hora en formato extendido con nombres completos.
     * 
     * **FORMATO**: "Nombre_Día, DD de Nombre_Mes de AAAA - HH:MM:SS"
     * 
     * @param dateTime Estructura DateTime a formatear
     * @return String con fecha/hora en formato extendido
     */
    String formatDateExtended(const DateTime& dateTime);
    
    // =========================================================================
    // Métodos de Gestión de Cache
    // =========================================================================
    
    /**
     * @brief Invalida el cache forzando nueva lectura del RTC.
     * 
     * **USO**: Llamar cuando se sabe que el RTC ha sido modificado
     * externamente (ej: después de setDateTime).
     */
    void invalidateCache();
    
    // =========================================================================
    // Métodos de Diagnóstico
    // =========================================================================
    
    /**
     * @brief Obtiene información de diagnóstico del módulo.
     * 
     * **CONCEPTO DE OBSERVABILIDAD**: Proporciona visibilidad del estado
     * interno para debugging y monitoreo del sistema.
     * 
     * @return String con información detallada de diagnóstico
     */
    String getDiagnosticInfo();
};

#endif // __GET_DATE_H__
