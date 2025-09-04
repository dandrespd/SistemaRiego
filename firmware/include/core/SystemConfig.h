// SystemConfig.h  — Single source of truth for hardware pin mapping
// Generated/updated by DeepSeek agent: apply carefully and run tests.
// Sections: BOOT SAFETY | I2C & DISPLAY | RTC (DS1302) | ANALOG SENSORS (ADC1) |
//           DIGITAL SENSORS | RELAYS | SERVOS (PWM) | UTIL / NOTES
// IMPORTANT: Do not use strapping pins (0,2,5,12,15) or flash pins (6..11).
// See Espressif docs for ADC and strapping pins.

#pragma once
#include <stdint.h>

namespace HardwareConfig {

  // ---------- BOOT / RESERVED (notes) ----------
  // Avoid: GPIO0, GPIO2, GPIO5, GPIO12, GPIO15 (strapping pins)
  // Avoid: GPIO6..GPIO11 (flash/PSRAM)
  // Keep UART0 (TX0/RX0 = GPIO1/GPIO3) free if you rely on Serial monitor.

  // ---------- I2C (common) ----------
  // Default I2C pins on many ESP32 devkits (can be remapped explicitly with Wire.begin)
  constexpr uint8_t I2C_SDA = 21; // Default SDA — typical on DOIT DEVKIT V1
  constexpr uint8_t I2C_SCL = 22; // Default SCL — typical on DOIT DEVKIT V1
  // LCD: I2C module (PCF8574/expander). Typical addresses: 0x27 or 0x3F — run scanner to confirm.
  constexpr uint8_t LCD_I2C_ADDR = 0x3F; // default guess; use I2C scanner in tests

  // ---------- RTC: DS1302 (3-wire: CE / SCLK / IO) ----------
  // DS1302 is NOT I2C. IO must be bidirectional (do NOT place on input-only pins 34..39).
  // We pick bidirectional non-strap pins to be safe.
  constexpr uint8_t RTC_CE   = 23; // Chip Enable (digital output)
  constexpr uint8_t RTC_SCLK = 19; // Serial clock (digital output)
  constexpr uint8_t RTC_IO   = 18; // Bidirectional data (digital I/O)

  // ---------- ANALOG SENSORS (use ADC1 pins only; ADC2 unreliable with Wi-Fi) ----------
  // ADC1: GPIO32..GPIO39 are safe for analog with Wi-Fi active. (See docs).
  // Mapping chosen to preserve ADC1 channels and to use input-only pins for raw sensors.
  constexpr uint8_t SOIL_MOISTURE_1 = 34; // ADC1_CH6 — input-only (no internal pullup)
  constexpr uint8_t SOIL_MOISTURE_2 = 35; // ADC1_CH7 — input-only (no internal pullup)
  constexpr uint8_t BATTERY_VOLTAGE = 32; // ADC1_CH4 — use voltage divider
  constexpr uint8_t LIGHT_SENSOR     = 36; // Moved to ADC1_CH0 (previously FLOAT_LEVEL_1)

  // ---------- DIGITAL SENSORS (float switches / reed) ----------
  // Input-only pins (34..39) have NO internal pull-ups — use external resistors if chosen.
  constexpr uint8_t FLOAT_LEVEL_1 = 33; // Moved from 36 to resolve conflict
  constexpr uint8_t FLOAT_LEVEL_2 = 39; // input-only (use external pull-up ~10k)

  // ---------- RELAYS / ACTUATORS (digital outputs; use transistors/driver if required) ----------
  constexpr uint8_t RELAY_VALVE_MAIN = 4; 
  constexpr uint8_t RELAY_PUMP       = 13;
  constexpr uint8_t RELAY_AUX        = 27; 

  // ---------- SERVOS (PWM via LEDC) ----------
  // Use non-input-only pins. Provide up to 4 PWM outputs
  constexpr uint8_t SERVO_PINS[] = {25, 26, 14, 33}; // Kept 33 for servo (now free)
  constexpr uint8_t NUM_SERVOS = sizeof(SERVO_PINS)/sizeof(SERVO_PINS[0]);

  // ---------- DEBUG / COMMUNICATION ----------
  // Leave UART0 (GPIO1 TX0 / GPIO3 RX0) free for Serial monitor debugging.
  // If you need a second UART, use Serial1/Serial2 and map to safe pins.

  // ---------- HARDWARE NOTES ----------
  // - Input-only pins (34..39) have NO internal pull-ups/pull-downs — add external resistors.
  // - Avoid reassigning strapping pins or flash pins; check hardware wiring before flashing.
  // - When changing pin assignments, update any legacy SET_PIN.h to include this header and map old names.
  // - After updating, run the I2C scanner and analog-read tests described in README/tests.

} // namespace HardwareConfig
