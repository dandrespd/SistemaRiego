#!/bin/bash
# Realiza seguimiento detallado de carga por USB con ESP32
set -e

# Añadir pip al PATH
export PATH="$PATH:/home/dandr/.local/bin"

echo "⚡ Iniciando seguimiento de carga USB"
echo "====================================="

# Detectar puerto USB automáticamente
PORT=$(ls /dev/ttyUSB* 2>/dev/null | head -n1)
[ -z "$PORT" ] && PORT=$(ls /dev/ttyACM* 2>/dev/null | head -n1)

if [ -z "$PORT" ]; then
  echo "❌ Error: No se detectaron dispositivos USB conectados"
  exit 1
fi

# Verificar permisos para el puerto detectado
if [ ! -r "$PORT" ] || [ ! -w "$PORT" ]; then
  echo "❌ Error: Permisos insuficientes para $PORT"
  echo "Por favor ejecuta manualmente: sudo chmod 0666 $PORT"
  exit 1
fi

echo "Dispositivo detectado: $PORT"
echo "---------------------------"

# Registrar información del chip usando el nuevo comando
echo "## Información del chip ESP32 ##" > flash_report.txt
/home/dandr/Escritorio/RiegoAuto/SistemaRiego/platformio_env/bin/esptool --port $PORT chip_id >> flash_report.txt 2>&1
echo "" >> flash_report.txt

# Iniciar carga con seguimiento detallado usando ruta completa de PlatformIO
echo "## Proceso de Carga ##" >> flash_report.txt
/home/dandr/Escritorio/RiegoAuto/SistemaRiego/platformio_env/bin/platformio run -e esp32doit-usb -t upload --upload-port $PORT | tee -a flash_report.txt

# Verificar resultado
if [ ${PIPESTATUS[0]} -eq 0 ]; then
  echo "✅ Carga completada exitosamente"
  echo "Puede ver el reporte completo en flash_report.txt"
else
  echo "❌ Error durante la carga"
  echo "Revise flash_report.txt para detalles"
fi

# Monitor de depuración post-carga
echo ""
echo "Para iniciar monitor serial:"
echo "platformio device monitor --port $PORT --baud 115200"
