/**
 * @file SET_DATE.h
 * @brief Clase de alto nivel para configurar la fecha y hora del RTC.
 * 
 * Se encarga de leer la entrada del usuario por el puerto serie, validarla
 * y, si es correcta, establecerla en el RTC a través de la interfaz IRTC.
 */
#ifndef __SET_DATE_H__
#define __SET_DATE_H__

#include <Arduino.h>
#include "IRTC.h"

class SetDate {
private:
    IRTC* rtc; // Puntero a un objeto RTC que cumple con la interfaz IRTC.
    
    // Funciones de ayuda internas.
    uint8_t parseDigits(char* str, uint8_t count);
    bool isValidDateTime(const DateTime& dt);

public:
    explicit SetDate(IRTC* rtc); // Constructor con inyección de dependencias.
    ~SetDate();

    void init(); // Inicializa el RTC a través de la interfaz.
    
    // Intenta leer y procesar la entrada del puerto serie para configurar la fecha.
    // Devuelve true si la configuración fue exitosa en esta llamada.
    bool setDateFromSerial();
};

#endif
