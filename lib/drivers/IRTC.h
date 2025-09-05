#ifndef __IRTC_H__
#define __IRTC_H__

/**
 * @file IRTC.h
 * @brief Interfaz genérica para módulos de Real Time Clock (RTC).
 * 
 * **CONCEPTO EDUCATIVO - INTERFACES Y ABSTRACCIÓN**:
 * Como profesor siempre enfatizo la importancia de las interfaces en programación.
 * Una interfaz es como un contrato que define qué operaciones debe soportar
 * cualquier implementación de RTC, sin importar el hardware específico.
 * 
 * **ANALOGÍA EDUCATIVA**: Es como tener un estándar universal para enchufes
 * eléctricos. No importa qué dispositivo conectes, si sigue la interfaz estándar,
 * funcionará correctamente. De la misma manera, cualquier RTC que implemente
 * esta interfaz podrá ser usado en nuestro sistema sin modificar el código principal.
 * 
 * **BENEFICIOS DE ESTA APROXIMACIÓN**:
 * 1. Intercambiabilidad: Fácil cambio entre diferentes chips RTC
 * 2. Testabilidad: Posibilidad de crear implementaciones mock para testing
 * 3. Extensibilidad: Agregar nuevos tipos de RTC sin cambiar código existente
 * 4. Claridad: Define claramente qué funcionalidades debe tener un RTC
 * 
 * @author Sistema de Riego Inteligente
 * @version 1.0 - Interfaz genérica para RTC
 * @date 2025
 */

// =============================================================================
// Includes necesarios
// =============================================================================
#include <Arduino.h>
#include <HardwareSerial.h>
#include "SystemConfig.h"

// =============================================================================
// Estructura de datos para fecha y hora
// =============================================================================

/**
 * @struct DateTime
 * @brief Estructura estándar para representar fecha y hora.
 * 
 * **CONCEPTO EDUCATIVO**: Esta estructura encapsula todos los componentes
 * de fecha y hora en un formato estándar. Es como tener un "contenedor"
 * organizado donde cada compartimento tiene un propósito específico.
 */
struct DateTime {
    uint8_t year;       // Año (formato YY, ej: 25 para 2025)
    uint8_t month;      // Mes (1-12)
    uint8_t day;        // Día del mes (1-31)
    uint8_t dayOfWeek;  // Día de la semana (1=Lunes, 7=Domingo)
    uint8_t hour;       // Hora (0-23)
    uint8_t minute;     // Minuto (0-59)
    uint8_t second;     // Segundo (0-59)
    
    /**
     * @brief Constructor por defecto que inicializa con valores seguros.
     */
    DateTime() : year(25), month(1), day(1), dayOfWeek(1), hour(0), minute(0), second(0) {}
    
    /**
     * @brief Constructor parametrizado para inicialización directa.
     */
    DateTime(uint8_t y, uint8_t mo, uint8_t d, uint8_t dow, uint8_t h, uint8_t mi, uint8_t s)
        : year(y), month(mo), day(d), dayOfWeek(dow), hour(h), minute(mi), second(s) {}
    
    /**
     * @brief Valida si la fecha/hora es lógicamente correcta.
     * 
     * **CONCEPTO EDUCATIVO**: La validación es crucial en sistemas embebidos
     * porque datos incorrectos pueden causar comportamientos impredecibles.
     * 
     * @return true si la fecha/hora es válida
     */
    bool isValid() const {
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
        if (hour > 23) return false;
        if (minute > 59) return false;
        if (second > 59) return false;
        if (dayOfWeek < 1 || dayOfWeek > 7) return false;
        
        // Validación específica de días por mes (simplificada)
        uint8_t daysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (day > daysInMonth[month - 1]) return false;
        
        return true;
    }
    
    /**
     * @brief Convierte la fecha/hora a string legible.
     * 
     * @return String en formato "DD/MM/YYYY HH:MM:SS"
     */
    String toString() const {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%02d/%02d/20%02d %02d:%02d:%02d",
                day, month, year, hour, minute, second);
        return String(buffer);
    }
    
    /**
     * @brief Convierte solo la fecha a string.
     * 
     * @return String en formato "DD/MM/YYYY"
     */
    String toDateString() const {
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%02d/%02d/20%02d", day, month, year);
        return String(buffer);
    }
    
    /**
     * @brief Convierte solo la hora a string.
     * 
     * @return String en formato "HH:MM:SS"
     */
    String toTimeString() const {
        char buffer[10];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hour, minute, second);
        return String(buffer);
    }
};

// =============================================================================
// Interfaz IRTC (Abstract Base Class)
// =============================================================================

/**
 * @class IRTC
 * @brief Interfaz abstracta para módulos de Real Time Clock.
 * 
 * **CONCEPTO EDUCATIVO - POLIMORFISMO**:
 * Esta clase define la "forma" que debe tener cualquier implementación de RTC.
 * Es como crear un molde que garantiza que todas las implementaciones tengan
 * las mismas funcionalidades básicas, pero cada una puede implementarlas
 * de manera diferente según su hardware específico.
 * 
 * **PATRÓN DE DISEÑO**: Esta clase implementa el patrón "Template Method"
 * y "Strategy Pattern", donde definimos la interfaz común pero permitimos
 * implementaciones específicas para cada tipo de hardware.
 */
class IRTC {
public:
    /**
     * @brief Destructor virtual para polimorfismo correcto.
     * 
     * **CONCEPTO IMPORTANTE**: En C++, cuando usamos herencia polimórfica,
     * necesitamos un destructor virtual para asegurar que se llame al
     * destructor correcto de la clase derivada.
     */
    virtual ~IRTC() = default;
    
    // =========================================================================
    // Métodos abstractos que deben implementar las clases derivadas
    // =========================================================================
    
    /**
     * @brief Inicializa el módulo RTC.
     * 
     * Este método debe configurar la comunicación con el chip RTC específico
     * y verificar que esté funcionando correctamente.
     * 
     * @return true si la inicialización fue exitosa
     */
    virtual bool init() = 0;
    
    /**
     * @brief Obtiene la fecha y hora actual del RTC.
     * 
     * **CONCEPTO EDUCATIVO**: Este método debe leer los registros del RTC
     * y convertir los datos binarios a una estructura DateTime legible.
     * 
     * @param dateTime Puntero donde almacenar la fecha/hora leída
     * @return true si la lectura fue exitosa
     */
    virtual bool getDateTime(DateTime* dateTime) = 0;
    
    /**
     * @brief Establece la fecha y hora en el RTC.
     * 
     * **IMPORTANTE**: Este método modifica los registros del RTC, por lo que
     * debe validar los datos antes de escribirlos para evitar corrupción.
     * 
     * @param dateTime Fecha/hora a escribir en el RTC
     * @return true si la escritura fue exitosa
     */
    virtual bool setDateTime(const DateTime& dateTime) = 0;
    
    /**
     * @brief Verifica si el RTC está funcionando (reloj no detenido).
     * 
     * Muchos RTCs tienen un bit de estado que indica si el reloj se ha
     * detenido (por ejemplo, por falta de batería de respaldo).
     * 
     * @return true si el RTC está detenido o no funciona
     */
    virtual bool isHalted() = 0;
    
    /**
     * @brief Reinicia/habilita el funcionamiento del RTC.
     * 
     * Útil para reactivar un RTC que se ha detenido o para inicializar
     * un RTC nuevo que viene detenido de fábrica.
     * 
     * @return true si la operación fue exitosa
     */
    virtual bool start() = 0;
    
    /**
     * @brief Detiene el funcionamiento del RTC.
     * 
     * Puede ser útil para conservar batería en aplicaciones de ultra-bajo consumo.
     * 
     * @return true si la operación fue exitosa
     */
    virtual bool stop() = 0;
    
    // =========================================================================
    // Métodos virtuales con implementación por defecto
    // =========================================================================
    
    /**
     * @brief Obtiene información de diagnóstico del RTC.
     * 
     * **IMPLEMENTACIÓN POR DEFECTO**: Proporciona información básica.
     * Las clases derivadas pueden sobrescribir este método para proporcionar
     * información más específica de su hardware.
     * 
     * @return String con información de diagnóstico
     */
    virtual String getDiagnosticInfo() {
        DateTime current;
        bool isWorking = !isHalted();
        bool canRead = getDateTime(&current);
        
        String info = "RTC Status:\n";
        info += "  Working: " + String(isWorking ? "Yes" : "No") + "\n";
        info += "  Readable: " + String(canRead ? "Yes" : "No") + "\n";
        
        if (canRead && current.isValid()) {
            info += "  Current Time: " + current.toString() + "\n";
        } else {
            info += "  Current Time: Invalid or unreadable\n";
        }
        
        return info;
    }
    
    /**
     * @brief Valida una fecha/hora antes de escribirla al RTC.
     * 
     * **VALIDACIÓN CENTRALIZADA**: Este método proporciona validación
     * estándar que pueden usar todas las implementaciones.
     * 
     * @param dateTime Fecha/hora a validar
     * @return true si es válida para escribir al RTC
     */
    virtual bool validateDateTime(const DateTime& dateTime) {
        return dateTime.isValid();
    }
    
    /**
     * @brief Imprime la fecha/hora actual al puerto serie.
     * 
     * Método de conveniencia para debugging y monitoreo.
     */
    virtual void printCurrentDateTime() {
        DateTime current;
        if (getDateTime(&current)) {
            DEBUG_PRINTLN("RTC: " + current.toString());
        } else {
            DEBUG_PRINTLN("RTC: Error reading date/time");
        }
    }
    
    // =========================================================================
    // Métodos de utilidad estática
    // =========================================================================
    
    /**
     * @brief Convierte string de fecha/hora a estructura DateTime.
     * 
     * **FORMATO ESPERADO**: "YY-MM-DD-W-HH-MM-SS" donde W es día de la semana
     * Ejemplo: "25-12-25-3-14-30-00" = 25 de diciembre de 2025, miércoles, 14:30:00
     * 
     * @param dateTimeStr String con fecha/hora a convertir
     * @param result Estructura donde almacenar el resultado
     * @return true si la conversión fue exitosa
     */
    static bool parseDateTime(const String& dateTimeStr, DateTime& result) {
        // Implementación de parsing robusto
        int values[7];
        int valueIndex = 0;
        String currentNumber = "";
        
        for (size_t i = 0; i < dateTimeStr.length() && valueIndex < 7; i++) {
            char c = dateTimeStr.charAt(i);
            
            if (c >= '0' && c <= '9') {
                currentNumber += c;
            } else if (c == '-' || i == dateTimeStr.length() - 1) {
                if (currentNumber.length() > 0) {
                    values[valueIndex++] = currentNumber.toInt();
                    currentNumber = "";
                }
            }
        }
        
        // Verificar que tenemos todos los valores necesarios
        if (valueIndex < 7) {
            return false;
        }
        
        // Crear estructura DateTime
        result = DateTime(values[0], values[1], values[2], values[3], 
                         values[4], values[5], values[6]);
        
        return result.isValid();
    }
    
    /**
     * @brief Calcula el día de la semana para una fecha dada.
     * 
     * **ALGORITMO EDUCATIVO**: Implementa el algoritmo de Zeller para calcular
     * el día de la semana. Es un ejemplo perfecto de cómo las matemáticas
     * se aplican en la programación práctica.
     * 
     * @param year Año (formato completo, ej: 2025)
     * @param month Mes (1-12)
     * @param day Día (1-31)
     * @return Día de la semana (1=Lunes, 7=Domingo)
     */
    static uint8_t calculateDayOfWeek(uint16_t year, uint8_t month, uint8_t day) {
        // Algoritmo de Zeller (simplificado)
        if (month < 3) {
            month += 12;
            year--;
        }
        
        int k = year % 100;
        int j = year / 100;
        
        int h = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) - 2 * j) % 7;
        
        // Convertir a formato Lunes=1, Domingo=7
        return ((h + 5) % 7) + 1;
    }
};

// =============================================================================
// Macros de utilidad para RTC
// =============================================================================

/**
 * **MACROS EDUCATIVAS**: Estas macros simplifican operaciones comunes con RTC
 * y proporcionan una interfaz más limpia para el código de aplicación.
 */

// Macro para verificar si un RTC está disponible y funcionando
#define RTC_IS_AVAILABLE(rtc) ((rtc) != nullptr && !(rtc)->isHalted())

// Macro para obtener fecha/hora de manera segura
#define RTC_GET_SAFE(rtc, dateTime) \
    (RTC_IS_AVAILABLE(rtc) ? (rtc)->getDateTime(&dateTime) : false)

// Macro para imprimir fecha/hora con prefijo personalizado
#define RTC_PRINT_WITH_PREFIX(rtc, prefix) \
    do { \
        DateTime dt; \
        if (RTC_GET_SAFE(rtc, dt)) { \
            DEBUG_PRINTLN(String(prefix) + dt.toString()); \
        } else { \
            DEBUG_PRINTLN(String(prefix) + "RTC no disponible"); \
        } \
    } while(0)

#endif // __IRTC_H__
