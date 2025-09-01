/**
 * @file SET_DATE.cpp
 * @brief Implementación de la clase SetDate.
 */
#include "SET_DATE.h"
#include <Arduino.h>

// Constructor: Recibe y almacena un puntero a un objeto compatible con IRTC.
SetDate::SetDate(IRTC* rtc) : rtc(rtc) {}

// Destructor: No requiere acciones especiales.
SetDate::~SetDate() {}

// init: Llama al método de inicialización del RTC a través de la interfaz.
void SetDate::init() {
    rtc->init();
}

// parseDigits: Función de ayuda para convertir una subcadena de caracteres a un número.
uint8_t SetDate::parseDigits(char* str, uint8_t count) {
    uint8_t val = 0;
    while (count-- > 0) {
        val = (val * 10) + (*str++ - '0');
    }
    return val;
}

// isValidDateTime: Valida que todos los campos de la estructura DateTime sean lógicos y correctos.
bool SetDate::isValidDateTime(const DateTime& dt) {
    if (dt.year > 99) { return false; }
    if (dt.month < 1 || dt.month > 12) { return false; }
    if (dt.day < 1 || dt.day > 31) { return false; } // Simplificado, la biblioteca del RTC maneja días por mes.
    if (dt.dayOfWeek < 1 || dt.dayOfWeek > 7) { return false; }
    if (dt.hour > 23) { return false; }
    if (dt.minute > 59) { return false; }
    if (dt.second > 59) { return false; }
    return true;
}

// setDateFromSerial: Lógica no bloqueante para leer del puerto serie.
bool SetDate::setDateFromSerial() {
    static char buffer[14];
    static uint8_t char_idx = 0;

    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (char_idx > 0) {
                buffer[char_idx] = '\0';
                
                if (char_idx != 13) {
                    Serial.println("Error: Entrada inválida. Se esperaban 13 dígitos.");
                    char_idx = 0;
                    return false;
                }

                DateTime dt;
                dt.year = parseDigits(buffer, 2);
                dt.month = parseDigits(buffer + 2, 2);
                dt.day = parseDigits(buffer + 4, 2);
                dt.dayOfWeek = parseDigits(buffer + 6, 1);
                dt.hour = parseDigits(buffer + 7, 2);
                dt.minute = parseDigits(buffer + 9, 2);
                dt.second = parseDigits(buffer + 11, 2);

                char_idx = 0;
                if (isValidDateTime(dt)) {
                    rtc->setDateTime(dt);
                    return true;
                } else {
                    Serial.println("Error: La fecha u hora introducida no es válida.");
                    return false;
                }
            }
        } else if (isDigit(c)) {
            if (char_idx < 13) {
                buffer[char_idx++] = c;
            }
            // Si se reciben más de 13 dígitos, se ignoran para evitar desbordamiento.
        }
    }
    return false;
}
