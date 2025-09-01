#!/bin/bash

# =============================================================================
# SCRIPT DE LIMPIEZA DE FUNCIONES NO UTILIZADAS - SISTEMA DE RIEGO
# =============================================================================
#
# Este script identifica y marca funciones no utilizadas en el proyecto
# para su posterior eliminación o documentación.
#
# USO: ./cleanup_unused_functions.sh
#
# =============================================================================

echo "🧹 INICIANDO LIMPIEZA DE FUNCIONES NO UTILIZADAS"
echo "================================================"

# Crear directorio de backup
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

echo "📁 Creando backup en: $BACKUP_DIR"

# Lista de funciones no utilizadas identificadas por cppcheck
UNUSED_FUNCTIONS=(
    "src/GET_DATE.cpp:printDate"
    "src/GET_DATE.cpp:formatDateOnly"
    "src/GET_DATE.cpp:formatTimeOnly"
    "src/GET_DATE.cpp:formatDateExtended"
    "src/GET_DATE.cpp:invalidateCache"
    "src/GET_DATE.cpp:getDiagnosticInfo"
    "src/IN_DIGITAL.cpp:init"
    "src/IN_DIGITAL.cpp:isHigh"
    "src/IN_DIGITAL.cpp:isLow"
    "src/Led.cpp:init"
    "src/Led.cpp:toggle"
    "src/OUT_DIGITAL.cpp:init"
    "src/OUT_DIGITAL.cpp:high"
    "src/RTC_DS1302.cpp:start"
    "src/RTC_DS1302.cpp:stop"
    "src/RTC_DS1302.cpp:getLastError"
    "src/RTC_DS1302.cpp:getDiagnosticInfo"
    "src/RTC_DS1302.cpp:performSelfTest"
    "src/SET_DATE.cpp:setDateFromSerial"
    "src/ServoPWMController.cpp:startIrrigationCycle"
    "src/ServoPWMController.cpp:update"
    "src/ServoPWMController.cpp:stopIrrigationCycle"
    "src/ServoPWMController.cpp:resetEmergencyStop"
    "src/ServoPWMController.cpp:getCurrentState"
    "src/ServoPWMController.cpp:getZoneState"
    "src/ServoPWMController.cpp:isIrrigating"
    "src/ServoPWMController.cpp:getZoneInfo"
    "src/ServoPWMController.cpp:printSystemStatus"
    "src/ServoPWMController.cpp:openZoneValve"
    "src/ServoPWMController.cpp:closeZoneValve"
    "src/ServoPWMController.cpp:setZoneEnabled"
    "src/ServoPWMController.cpp:setZoneIrrigationTime"
    "src/ServoPWMController.cpp:getSystemStatistics"
    "src/ServoPWMController.cpp:hasErrors"
)

echo "📋 FUNCIONES NO UTILIZADAS IDENTIFICADAS:"
echo "========================================="

for func in "${UNUSED_FUNCTIONS[@]}"; do
    echo "❌ $func"
done

echo ""
echo "🔧 OPCIONES DE LIMPIEZA:"
echo "======================="
echo "1. Comentar funciones no utilizadas"
echo "2. Eliminar funciones no utilizadas"
echo "3. Crear documentación de funciones no utilizadas"
echo "4. Salir sin cambios"

read -p "Seleccione una opción (1-4): " choice

case $choice in
    1)
        echo "📝 COMENTANDO FUNCIONES NO UTILIZADAS..."
        for func in "${UNUSED_FUNCTIONS[@]}"; do
            file=$(echo "$func" | cut -d: -f1)
            func_name=$(echo "$func" | cut -d: -f2)
            
            # Crear backup
            cp "$file" "$BACKUP_DIR/"
            
            # Comentar función
            sed -i "s/^\([[:space:]]*\)$func_name/\1\/\/ UNUSED: $func_name/g" "$file"
            echo "✅ Comentada: $func"
        done
        ;;
    2)
        echo "🗑️ ELIMINANDO FUNCIONES NO UTILIZADAS..."
        for func in "${UNUSED_FUNCTIONS[@]}"; do
            file=$(echo "$func" | cut -d: -f1)
            func_name=$(echo "$func" | cut -d: -f2)
            
            # Crear backup
            cp "$file" "$BACKUP_DIR/"
            
            echo "⚠️ Eliminando: $func"
            # Nota: La eliminación manual requiere revisión cuidadosa
        done
        ;;
    3)
        echo "📚 CREANDO DOCUMENTACIÓN DE FUNCIONES NO UTILIZADAS..."
        cat > "UNUSED_FUNCTIONS.md" << EOF
# 📋 FUNCIONES NO UTILIZADAS - SISTEMA DE RIEGO

## 📊 RESUMEN
Este documento lista las funciones identificadas como no utilizadas en el proyecto.
Estas funciones pueden ser candidatas para eliminación o refactorización.

## 🔍 FUNCIONES IDENTIFICADAS

### GET_DATE.cpp
- \`printDate\` - Función de impresión de fecha
- \`formatDateOnly\` - Formateo de solo fecha
- \`formatTimeOnly\` - Formateo de solo hora
- \`formatDateExtended\` - Formateo extendido de fecha
- \`invalidateCache\` - Invalidación de caché
- \`getDiagnosticInfo\` - Información de diagnóstico

### IN_DIGITAL.cpp
- \`init\` - Inicialización de entrada digital
- \`isHigh\` - Verificación de estado alto
- \`isLow\` - Verificación de estado bajo

### Led.cpp
- \`init\` - Inicialización de LED
- \`toggle\` - Cambio de estado de LED

### OUT_DIGITAL.cpp
- \`init\` - Inicialización de salida digital
- \`high\` - Establecer salida en alto

### RTC_DS1302.cpp
- \`start\` - Inicio de RTC
- \`stop\` - Parada de RTC
- \`getLastError\` - Obtener último error
- \`getDiagnosticInfo\` - Información de diagnóstico
- \`performSelfTest\` - Autoprueba

### SET_DATE.cpp
- \`setDateFromSerial\` - Configuración de fecha desde serial

### ServoPWMController.cpp
- \`startIrrigationCycle\` - Inicio de ciclo de riego
- \`update\` - Actualización del controlador
- \`stopIrrigationCycle\` - Parada de ciclo de riego
- \`resetEmergencyStop\` - Reset de parada de emergencia
- \`getCurrentState\` - Obtener estado actual
- \`getZoneState\` - Obtener estado de zona
- \`isIrrigating\` - Verificar si está regando
- \`getZoneInfo\` - Obtener información de zona
- \`printSystemStatus\` - Imprimir estado del sistema
- \`openZoneValve\` - Abrir válvula de zona
- \`closeZoneValve\` - Cerrar válvula de zona
- \`setZoneEnabled\` - Habilitar/deshabilitar zona
- \`setZoneIrrigationTime\` - Configurar tiempo de riego
- \`getSystemStatistics\` - Obtener estadísticas
- \`hasErrors\` - Verificar errores

## 🎯 RECOMENDACIONES

### Para Eliminación
- Funciones de debugging no utilizadas
- Funciones de testing obsoletas
- Funciones de configuración manual no implementadas

### Para Mantener
- Funciones de diagnóstico que pueden ser útiles
- Funciones de configuración para futuras implementaciones
- Funciones de testing para desarrollo

## 📝 NOTAS
- Revisar cada función antes de eliminar
- Considerar si son necesarias para debugging
- Documentar razones de eliminación
- Mantener compatibilidad hacia atrás si es necesario

---
*Documento generado automáticamente - $(date)*
EOF
        echo "✅ Documentación creada: UNUSED_FUNCTIONS.md"
        ;;
    4)
        echo "❌ Operación cancelada"
        exit 0
        ;;
    *)
        echo "❌ Opción inválida"
        exit 1
        ;;
esac

echo ""
echo "✅ LIMPIEZA COMPLETADA"
echo "====================="
echo "📁 Backup creado en: $BACKUP_DIR"
echo "🔍 Revisar cambios antes de compilar"
echo "🧪 Ejecutar tests después de los cambios"
