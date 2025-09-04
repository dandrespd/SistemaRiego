#ifndef ARDUINO_MINIMAL_H
#define ARDUINO_MINIMAL_H

#include <stdint.h>

// Minimal Arduino-like definitions
#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

typedef uint8_t byte;

void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
unsigned long millis() { return 0; }
void delay(unsigned long ms) {}

#endif
