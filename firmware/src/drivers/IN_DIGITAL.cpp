#include "IN_DIGITAL.h"

// Constructor: Almacena el pin especificado.
InDigital::InDigital(uint8_t pin) : pin(pin) {}

// Destructor: No requiere acciones especiales.
InDigital::~InDigital() {}

// init: Configura el pin GPIO como una entrada digital.
void InDigital::init() {
    pinMode(pin, INPUT);
}

// isHigh: Comprueba si la entrada está en estado ALTO.
bool InDigital::isHigh() {
    return (digitalRead(pin) == HIGH);
}

// isLow: Comprueba si la entrada está en estado BAJO.
bool InDigital::isLow() {
    return (digitalRead(pin) == LOW);
}
