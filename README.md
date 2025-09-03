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
