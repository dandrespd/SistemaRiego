/**
 * @file IN_DIGITAL.h
 * @brief Abstracción para una entrada digital genérica (ej. un sensor).
 * 
 * Encapsula la lógica para leer el estado de un pin configurado como entrada.
 */
#ifndef __IN_DIGITAL_H__
#define __IN_DIGITAL_H__

#include <Arduino.h>

class InDigital {
private:
    uint8_t pin; // Pin GPIO de la entrada digital.

public:
    explicit InDigital(uint8_t pin); // Constructor que recibe el pin.
    ~InDigital(); // Destructor.

    void init();      // Configura el pin como entrada.
    bool isHigh();    // Devuelve true si la entrada está en estado ALTO.
    bool isLow();     // Devuelve true si la entrada está en estado BAJO.
};

#endif
