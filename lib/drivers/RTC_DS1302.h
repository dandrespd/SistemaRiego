#ifndef __RTC_DS1302_H__
#define __RTC_DS1302_H__

/**
 * @file RTC_DS1302.h
 * @brief Implementación avanzada de RTC DS1302 que cumple la interfaz IRTC.
 * 
 * **CONCEPTO EDUCATIVO - EVOLUCIÓN ARQUITECTÓNICA**:
 * Como profesor siempre enseño que el código debe evolucionar manteniendo
 * compatibilidad hacia atrás pero adoptando mejores prácticas. Esta clase
 * demuestra esa evolución: de un wrapper simple a una implementación completa
 * que cumple estándares profesionales.
 * 
 * **BENEFICIOS DE ESTA ARQUITECTURA**:
 * 1. **Interface Compliance**: Cumple exactamente el contrato IRTC
 * 2. **Error Handling**: Manejo robusto de errores con códigos específicos
 * 3. **Self-Diagnostic**: Capacidades de autodiagnóstico integradas
 * 4. **Data Validation**: Validación exhaustiva de datos de entrada
 * 5. **Professional Logging**: Sistema de logging para debugging
 * 
 * **PATRÓN DE DISEÑO**: Esta clase implementa múltiples patrones:
 * - Adapter: Adapta biblioteca Ds1302 a interfaz estándar
 * - Template Method: Define flujo de operaciones con puntos de extensión
 * - Error Object: Encapsula información de errores de manera estructurada
 * 
 * @author Sistema de Riego Inteligente
 * @version 2.0 - Implementación profesional conforme a IRTC
 * @date 2025
 */

// =============================================================================
// Includes necesarios
// =============================================================================
#include <Ds1302.h>       // Biblioteca externa para chip DS1302
#include "IRTC.h"         // Interfaz genérica que implementamos

// =============================================================================
// Enumeraciones y Tipos Específicos
// =============================================================================

/**
 * @class RTC_DS1302
 * @brief Implementación completa de RTC DS1302 que cumple la interfaz IRTC.
 * 
 * **CONCEPTO EDUCATIVO - IMPLEMENTATION INHERITANCE**:
 * Esta clase "hereda" de IRTC, lo que significa que DEBE implementar todos
 * los métodos que la interfaz define. Es como firmar un contrato legal:
 * prometes proporcionar ciertos servicios de manera específica.
 * 
 * **VENTAJAS DE ESTA APROXIMACIÓN**:
 * - **Intercambiabilidad**: Cualquier código que use IRTC puede usar esta clase
 * - **Polimorfismo**: Se puede usar la misma interfaz para diferentes chips RTC
 * - **Testing**: Fácil crear mocks para pruebas unitarias
 * - **Mantenibilidad**: Cambios internos no afectan código cliente
 * 
 * **FUNCIONALIDADES AVANZADAS**:
 * - Manejo robusto de errores con códigos detallados
 * - Validación de datos antes de escritura
 * - Autodiagnóstico con reportes detallados
 * - Logging profesional para debugging
 * - Auto-tests para verificación de hardware
 * 
 * **EJEMPLO DE USO AVANZADO**:
 * ```cpp
 * RTC_DS1302 rtc(RST_PIN, SCLK_PIN, IO_PIN);
 * 
 * // Inicialización robusta con verificación
 * if (!rtc.init()) {
 *     Serial.println("Error: " + rtc.errorToString(rtc.getLastError()));
 *     return;
 * }
 * 
 * // Verificación de estado
 * if (rtc.isHalted()) {
 *     Serial.println("RTC detenido - configuración requerida");
 *     DateTime defaultTime(25, 1, 1, 1, 12, 0, 0);
 *     rtc.setDateTime(defaultTime);
 * }
 * 
 * // Uso normal
 * DateTime current;
 * if (rtc.getDateTime(&current)) {
 *     Serial.println("Hora: " + current.toString());
 * } else {
 *     Serial.println("Error leyendo RTC");
 * }
 * 
 * // Diagnóstico detallado
 * Serial.println(rtc.getDiagnosticInfo());
 * ```
 */
class RTC_DS1302 : public IRTC {
public:
    
    // =========================================================================
    // Enumeraciones Específicas del DS1302
    // =========================================================================
    
    /**
     * @enum RTCError
     * @brief Códigos de error específicos para operaciones del DS1302.
     * 
     * **CONCEPTO EDUCATIVO**: Los códigos de error estructurados son fundamentales
     * en sistemas profesionales. En lugar de simplemente "algo falló", proporcionamos
     * información específica que permite diagnóstico rápido y acciones correctivas.
     */
    enum class RTCError {
        NONE = 0,                        // Sin errores
        NOT_INITIALIZED,                 // RTC no inicializado
        COMMUNICATION_FAILED,            // Fallo de comunicación con chip
        INVALID_PARAMETER,               // Parámetro inválido pasado a función
        INVALID_DATA,                    // Datos leídos del chip son inválidos
        CLOCK_HALTED,                    // Reloj del chip está detenido
        WRITE_VERIFICATION_FAILED,       // Escritura no se pudo verificar
        INITIALIZATION_FAILED,           // Inicialización del chip falló
        NOT_SUPPORTED                    // Operación no soportada por hardware
    };
    
    // =========================================================================
    // Constructor y Destructor
    // =========================================================================
    
    /**
     * @brief Constructor que establece configuración de pines.
     * 
     * **CONCEPTO EDUCATIVO**: El constructor prepara el objeto pero NO realiza
     * operaciones de hardware. Esto permite crear el objeto incluso si el hardware
     * no está disponible, y diferir la inicialización hasta el momento apropiado.
     * 
     * @param rst_pin Pin GPIO conectado a RST del DS1302
     * @param sclk_pin Pin GPIO conectado a SCLK del DS1302
     * @param io_pin Pin GPIO conectado a IO/DAT del DS1302
     */
    explicit RTC_DS1302(uint8_t rst_pin, uint8_t sclk_pin, uint8_t io_pin);
    
    /**
     * @brief Destructor que asegura limpieza de recursos.
     */
    virtual ~RTC_DS1302();
    
    // =========================================================================
    // Implementación de la Interfaz IRTC (Métodos Obligatorios)
    // =========================================================================
    
    /**
     * @brief Inicializa el módulo RTC con verificación robusta.
     * 
     * **MEJORAS SOBRE VERSIÓN ANTERIOR**:
     * - Verificación de comunicación con chip
     * - Validación de datos leídos
     * - Manejo de excepciones
     * - Logging detallado de errores
     * 
     * @return true si inicialización fue exitosa, false en caso de error
     */
    bool init() override;
    
    /**
     * @brief Obtiene fecha/hora con validación robusta.
     * 
     * **MEJORAS IMPLEMENTADAS**:
     * - Verificación de parámetros de entrada
     * - Validación de datos leídos
     * - Manejo de errores de comunicación
     * - Logging detallado para debugging
     * 
     * @param dateTime Puntero donde almacenar fecha/hora leída
     * @return true si lectura fue exitosa, false en caso de error
     */
    bool getDateTime(DateTime* dateTime) override;
    
    /**
     * @brief Establece fecha/hora con validación y verificación.
     * 
     * **CARACTERÍSTICAS AVANZADAS**:
     * - Validación previa de datos
     * - Escritura al hardware
     * - Verificación post-escritura
     * - Rollback automático en caso de fallo
     * 
     * @param dateTime Fecha/hora a escribir en el RTC
     * @return true si escritura fue exitosa, false en caso de error
     */
    bool setDateTime(const DateTime& dateTime) override;
    
    /**
     * @brief Verifica si el reloj está detenido con diagnóstico.
     * 
     * @return true si el reloj está detenido, false si funciona normalmente
     */
    bool isHalted() override;
    
    /**
     * @brief Inicia el funcionamiento del RTC.
     * 
     * **IMPLEMENTACIÓN ESPECÍFICA**: Como el DS1302 no tiene comando start
     * explícito, usamos escritura de fecha válida para activar el reloj.
     * 
     * @return true si se pudo iniciar el RTC
     */
    bool start() override;
    
    /**
     * @brief Detiene el funcionamiento del RTC.
     * 
     * **LIMITACIÓN DE HARDWARE**: El DS1302 no soporta comando stop explícito.
     * 
     * @return false (operación no soportada)
     */
    bool stop() override;
    
    // =========================================================================
    // Métodos Específicos del DS1302
    // =========================================================================
    
    /**
     * @brief Obtiene el último error que ocurrió.
     * 
     * **CONCEPTO DE ERROR TRACKING**: Mantener registro del último error
     * permite diagnóstico detallado sin necesidad de excepciones.
     * 
     * @return Código del último error ocurrido
     */
    RTCError getLastError() const;
    
    /**
     * @brief Convierte código de error a descripción legible.
     * 
     * **CONCEPTO DE USER EXPERIENCE**: Los códigos numéricos son útiles para
     * programas, pero los humanos necesitan descripciones claras.
     * 
     * @param error Código de error a convertir
     * @return Descripción textual del error
     */
    static String errorToString(RTCError error);
    
    /**
     * @brief Genera reporte de diagnóstico completo.
     * 
     * **CONCEPTO DE OBSERVABILIDAD**: En sistemas críticos es fundamental
     * poder "ver adentro" del sistema para entender qué está pasando.
     * 
     * @return String con información detallada de diagnóstico
     */
    String getDiagnosticInfo() override;
    
    /**
     * @brief Realiza test completo del hardware RTC.
     * 
     * **CONCEPTO DE SELF-TESTING**: Los sistemas profesionales pueden
     * verificar su propio estado para detectar problemas tempranamente.
     * 
     * @return true si todos los tests pasaron exitosamente
     */
    bool performSelfTest();
    
private:
    // =========================================================================
    // Atributos Privados
    // =========================================================================
    
    Ds1302 rtc;                    // Instancia de biblioteca Ds1302
    bool isInitialized;            // Estado de inicialización
    RTCError lastError;            // Último error ocurrido
    
    // =========================================================================
    // Métodos Privados de Implementación
    // =========================================================================
    
    // (Los métodos privados se implementan en el archivo .cpp)
};

#endif // __RTC_DS1302_H__
