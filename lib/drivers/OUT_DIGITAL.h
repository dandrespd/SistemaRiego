/**
 * @file OUT_DIGITAL.h
 * @brief Abstracción para una salida digital genérica (ej. un relé o electroválvula).
 * 
 * Encapsula la lógica para controlar el estado de un pin configurado como salida.
 */
#ifndef __OUT_DIGITAL_H__
#define __OUT_DIGITAL_H__

#include <Arduino.h>

class OutDigital {
private:
    uint8_t pin; // Pin GPIO de la salida digital.

public:
    explicit OutDigital(uint8_t pin); // Constructor que recibe el pin.
    ~OutDigital(); // Destructor.

    void init();   // Configura el pin como salida.
    void high();   // Establece la salida en estado ALTO.
    void low();    // Establece la salida en estado BAJO.
};

#endif
