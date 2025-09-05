# Sistema de Riego Inteligente

## Estructura del Proyecto
```bash
├── src/              # Código fuente principal (C++/Arduino)
│   ├── core/         # Core del sistema
│   ├── drivers/      # Drivers de dispositivos
│   ├── network/      # Funciones de red
│   └── utils/        # Utilidades
├── lib/              # Librerías personalizadas
│   ├── drivers/      # Headers de drivers
│   └── utils/        # Headers de utilidades
├── data/             # Archivos para sistema de archivos SPIFFS/LittleFS
├── web/              # Aplicación web de control
└── test/             # Pruebas unitarias
```

## Prerrequisitos
- Node.js >= 18.x
- pnpm: Instalar globalmente con `npm install -g pnpm`
- PlatformIO para desarrollo del firmware

Si encuentra el error "pnpm command not found":
1. Verifique que Node.js esté instalado correctamente
2. Agregue pnpm al PATH: `export PATH="$PATH:$HOME/.npm-global/bin"` (Linux/macOS)
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

## Instalación
1. Clonar el repositorio:
   ```bash
   git clone https://github.com/dandrespd/SistemaRiego.git
   cd SistemaRiego
   ```

2. Configurar variables de entorno para la aplicación web:
   ```bash
   cd web
   cp .env.example .env
   # Editar .env con los detalles de tu ESP32
   ```
   
3. Instalar dependencias de la aplicación web:
   ```bash
   cd web
   pnpm install
   ```

4. Instalar dependencias del firmware (desde la raíz del proyecto):
   ```bash
   pio run
   ```

## Ejecutar Aplicación Web
```bash
cd web
pnpm dev
```

## Compilar y Subir Firmware
```bash
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

## Pruebas
### Ejecutar pruebas unitarias
```bash
pio test
```

### Prueba de I2C
Para dispositivos I2C como pantallas LCD:
```bash
cd test
pio run -e i2c-scanner -t upload
```

### Prueba de RTC
Para verificar el funcionamiento del reloj RTC:
```bash
cd test
pio run -e rtc-test -t upload
```

## Documentación Hardware

### Mapeo de Pines (ESP32 DOIT DEVKIT V1)
Hemos implementado un mapeo completo que evita GPIO problemáticos:

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

**ADC Guidance:**
- Always use ADC1 pins (32-39) for analog sensors to ensure reliable readings when WiFi is active
- ADC2 pins become unreliable when WiFi is active due to RF interference
- For more details, see [Espressif ADC2 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#adc-limitations-when-using-wi-fi)

### RTC DS1302 Wiring (Interfaz 3 hilos)
El módulo DS1302 usa una interfaz de 3 hilos diferente de los dispositivos I2C estándar:

| DS1302 Pin | ESP32 Pin (DOIT DEVKIT V1) | Description      |
|------------|----------------------------|------------------|
| CE         | GPIO25                     | Chip Enable      |
| I/O        | GPIO32                     | Data Line        |
| SCLK       | GPIO26                     | Serial Clock     |

**Note:** This is a 3-wire interface (CE/CLK/IO), not standard I2C. The connections listed above match the configuration in SystemConfig.h.

### Flasheo Inicial
Para flashear el dispositivo por primera vez usando USB:

```bash
# Linux/macOS
scripts/initial_flash.sh

# Windows
pio run -e esp32doit-usb -t upload
```

### Contribuciones
1. Crear nueva rama: `git checkout -b feature/tu-caracteristica`
2. Realizar cambios
3. Hacer commit y push: `git push origin feature/tu-caracteristica`
4. Crear pull request

### RTC Test
To verify RTC functionality, use the system's RTC test functionality:
```cpp
sistemaRiego->readRTC(); // Returns current datetime
