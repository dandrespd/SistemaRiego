#include <drivers/OUT_DIGITAL.h>

// Constructor: Almacena el pin especificado.
OutDigital::OutDigital(uint8_t pin) : pin(pin) {}

// Destructor: No requiere acciones especiales.
OutDigital::~OutDigital() {}

// init: Configura el pin GPIO como una salida y lo establece en BAJO por defecto.
void OutDigital::init() {
    pinMode(pin, OUTPUT);
    low(); // Iniciar con la salida desactivada.
}

// high: Establece el pin de salida en estado ALTO.
void OutDigital::high() {
    digitalWrite(pin, HIGH);
}

// low: Establece el pin de salida en estado BAJO.
void OutDigital::low() {
    digitalWrite(pin, LOW);
}
