#include "Led.h"

Led::Led(uint8_t pin){
    this->pin = pin;
}

Led::~Led(){

}

void Led::init(bool state){
    pinMode(pin, OUTPUT);
    if (state == HIGH)
        on();
    else
        off();
    
}

void Led::on(){
    digitalWrite(pin, HIGH);
}
void Led::off(){
    digitalWrite(pin, LOW);
}

void Led::toggle() {
    int current = digitalRead(pin);
    digitalWrite(pin, !current);
}
