# Smart Irrigation System

This project consists of:
- Firmware for ESP32-based irrigation controller
- Web application for monitoring and configuration

## Prerequisites
- Node.js >= 18.x
- pnpm: First install globally with `npm install -g pnpm`
- PlatformIO (for firmware development)

If you encounter "pnpm command not found" errors:
1. Make sure Node.js is properly installed
2. Add pnpm to your PATH: `export PATH="$PATH:$HOME/.npm-global/bin"` (Linux/macOS)
   or update system environment variables (Windows)

## Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/dandrespd/SistemaRiego.git
   cd SistemaRiego
   ```

2. Setup environment variables:
   ```bash
   cd web
   cp .env.example .env
   # Edit .env file with your ESP32 details
   ```
   
3. Setup web application:
   ```bash
   cd web
   pnpm install
   ```

3. Setup firmware dependencies:
   ```bash
   cd firmware
   pio run
   ```

## Running the Web Application
```bash
cd web
pnpm dev
```

## Building and Flashing Firmware
```bash
cd firmware
pio run -t upload
```

## Deployment to ESP32
To deploy the web application to your ESP32 device:
1. Create a `.env` file in the `web` directory (use the template):
   ```bash
   cp web/.env.example web/.env
   ```
2. Edit `web/.env` with your ESP32's IP address and settings
3. Build the web application:
   ```bash
   cd web
   pnpm run build
   ```
4. Run the deployment script:
   ```bash
   node scripts/deploy-to-esp32.js
   ```

## First Boot Process
On first boot, you'll be guided to:
1. Connect to WiFi
2. Change default credentials
3. Configure irrigation zones

## Contributing
1. Create a new branch: `git checkout -b feature/your-feature`
2. Make your changes
3. Commit and push: `git push origin feature/your-feature`
4. Create a pull request

## Hardware Documentation

### Pin Mapping (ESP32 DOIT DEVKIT V1)
We've implemented a comprehensive pin mapping that avoids problematic GPIOs and follows best practices:

```c++
namespace HardwareConfig {
  // I2C
  I2C_SDA = 21;
  I2C_SCL = 22;

  // RTC DS1302 (3-wire)
  RTC_CE = 23;   // Chip Enable
  RTC_SCLK = 19; // Serial Clock
  RTC_IO = 18;   // Data Line

  // Analog Sensors (ADC1 only)
  SOIL_MOISTURE_1 = 34;
  SOIL_MOISTURE_2 = 35;
  BATTERY_VOLTAGE = 32;

  // Digital Sensors
  FLOAT_LEVEL_1 = 36;
  FLOAT_LEVEL_2 = 39;

  // Relays
  RELAY_VALVE_MAIN = 4;
  RELAY_PUMP = 13;

  // Servos
  SERVO_PINS = {25, 26, 14, 33};
}
```

**Key Safety Measures:**
- Avoided all strapping pins (0,2,5,12,15)
- All analog sensors use ADC1 pins (safe with WiFi)
- Input-only pins (34-39) used for sensors with external pull-ups
- Flash pins (6-11) completely avoided

### RTC DS1302 Wiring (3-Wire Interface)
The DS1302 module uses a 3-wire interface that is different from standard I2C devices:

| DS1302 Pin | ESP32 Pin (DOIT DEVKIT V1) | Description      |
|------------|----------------------------|------------------|
| CE         | GPIO25                     | Chip Enable      |
| I/O        | GPIO32                     | Data Line        |
| SCLK       | GPIO26                     | Serial Clock     |

**Note:** This is a 3-wire interface (CE/CLK/IO), not standard I2C. The connections listed above match the configuration in SystemConfig.h.

### Initial Flashing
To flash the device for the first time using USB:

```bash
# For Linux/macOS
scripts/initial_flash.sh

# For Windows
pio run -e esp32doit-usb -t upload
```

### I2C Scan Tool
To help diagnose I2C devices (like LCD displays), we provide a scanning tool:

1. Upload the I2C scanner sketch:
```bash
cd firmware/tools
pio run -e esp32doit-usb -t upload --upload-port YOUR_PORT
```

2. Open the serial monitor to see results:
```bash
pio device monitor
```

3. If an LCD display is connected, it should appear at address `0x3F` (or similar).

### RTC Test
To verify RTC functionality, use the system's RTC test functionality:
```cpp
sistemaRiego->readRTC(); // Returns current datetime
```
