/**
 * @file Led.h
 * @brief Abstracción de hardware para controlar un diodo emisor de luz (LED).
 * 
 * Encapsula la lógica de bajo nivel (pinMode, digitalWrite) para manejar un LED,
 * ofreciendo una interfaz simple y clara (on, off, init).
 */
#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>

class Led {
private:
    uint8_t pin; // Pin GPIO donde está conectado el LED.

public:
    explicit Led(uint8_t pin); // Constructor que recibe el pin del LED.
    ~Led(); // Destructor.

    void init(bool initialState = LOW); // Inicializa el pin y establece un estado inicial.
    void on();  // Enciende el LED.
    void off(); // Apaga el LED.
    void toggle(); // Cambia el estado del LED
};

#endif
