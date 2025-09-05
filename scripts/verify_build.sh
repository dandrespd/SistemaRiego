#!/bin/bash
# Verifica compilación y carga del proyecto en ESP32
set -e

echo "🔍 Verificando estructura del proyecto..."
[ -d "firmware" ] || { echo "❌ Error: Directorio firmware no encontrado"; exit 1; }
[ -f "platformio.ini" ] || { echo "❌ Error: platformio.ini no encontrado"; exit 1; }

echo "⚙️ Ejecutando compilación..."
pio run

echo "✅ Compilación exitosa!"
echo ""
echo "Para cargar el firmware en el ESP32:"
echo "1. Conecte el ESP32 por USB"
echo "2. Ejecute: pio run --target upload"
echo ""
echo "Para monitorear el dispositivo:"
echo "platformio device monitor"
