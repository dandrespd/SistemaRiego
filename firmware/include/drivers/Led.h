/**
 * @file Led.h
 * @brief Abstracción de hardware para controlar un diodo emisor de luz (LED).
 * 
 * Encapsula la lógica de bajo nivel (pinMode, digitalWrite) para manejar un LED,
 * ofreciendo una interfaz simple y clara (encender, apagar, inicializar).
 */
#ifndef __LED_H__
#define __LED_H__

#include <cstdint> // Proporciona uint8_t

// Definir constantes para evitar dependencia de Arduino.h
constexpr bool LOW = 0;
constexpr bool HIGH = 1;

/**
 * @class Led
 * @brief Controlador para un LED conectado a un pin GPIO
 */
class Led {
private:
    uint8_t pin; ///< Pin GPIO donde está conectado el LED

public:
    /**
     * @brief Constructor que recibe el pin del LED
     * @param pin Número del pin GPIO
     */
    explicit Led(uint8_t pin);
    
    /**
     * @brief Destructor
     */
    ~Led();

    /**
     * @brief Inicializa el pin y establece un estado inicial
     * @param initialState Estado inicial del LED (HIGH o LOW)
     */
    void inicializar(bool estado_inicial = LOW);
    
    /**
     * @brief Enciende el LED
     */
    void encender();
    
    /**
     * @brief Apaga el LED
     */
    void apagar();
    
    /**
     * @brief Cambia el estado del LED (toggle)
     */
    void alternar();
};

#endif
