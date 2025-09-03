#include <Wire.h>
#include <Arduino.h>
void setup(){
  Serial.begin(115200);
  delay(2000);
  Wire.begin(/*SDA*/21, /*SCL*/22);
  Serial.println("I2C scan starting...");
  for (uint8_t addr=1; addr<127; addr++){
    Wire.beginTransmission(addr);
    if (Wire.endTransmission()==0){
      Serial.printf("Found I2C device at 0x%02X\n", addr);
      delay(2);
    }
  }
}
void loop(){ delay(10000); }
