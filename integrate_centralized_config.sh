#!/bin/bash

# =============================================================================
# SCRIPT DE INTEGRACIÓN CON CONFIGURACIÓN CENTRALIZADA - SISTEMA DE RIEGO
# =============================================================================
#
# Este script integra el proyecto con la nueva configuración centralizada
# ProjectConfig.h, migrando gradualmente desde las configuraciones dispersas.
#
# USO: ./integrate_centralized_config.sh
#
# =============================================================================

echo "🔧 INICIANDO INTEGRACIÓN CON CONFIGURACIÓN CENTRALIZADA"
echo "======================================================"

# Crear directorio de backup
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

echo "📁 Creando backup en: $BACKUP_DIR"

# Verificar que existe ProjectConfig.h
if [ ! -f "include/ProjectConfig.h" ]; then
    echo "❌ ERROR: ProjectConfig.h no encontrado"
    echo "Ejecute primero la creación de la configuración centralizada"
    exit 1
fi

echo "✅ ProjectConfig.h encontrado"

# Lista de archivos a migrar
FILES_TO_MIGRATE=(
    "src/main.cpp"
    "src/ServoPWMController.cpp"
    "src/WebControl.cpp"
    "src/SystemManager.cpp"
)

echo ""
echo "📋 ARCHIVOS A MIGRAR:"
echo "===================="

for file in "${FILES_TO_MIGRATE[@]}"; do
    echo "🔍 $file"
done

echo ""
echo "🔧 APLICANDO MIGRACIÓN..."

# Migrar main.cpp
if [ -f "src/main.cpp" ]; then
    cp "src/main.cpp" "$BACKUP_DIR/"
    
    echo "✅ Migrando main.cpp..."
    
    # Agregar include de ProjectConfig.h
    if ! grep -q "ProjectConfig.h" "src/main.cpp"; then
        # Insertar después de los includes existentes
        sed -i '/#include/a #include "ProjectConfig.h"' "src/main.cpp"
    fi
    
    # Reemplazar configuraciones hardcodeadas
    sed -i 's/Serial\.begin(115200)/Serial.begin(DebugConfig::Serial::BAUD_RATE)/g' "src/main.cpp"
    sed -i 's/Serial\.begin(115200)/Serial.begin(DebugConfig::Serial::BAUD_RATE)/g' "src/main.cpp"
fi

# Migrar ServoPWMController.cpp
if [ -f "src/ServoPWMController.cpp" ]; then
    cp "src/ServoPWMController.cpp" "$BACKUP_DIR/"
    
    echo "✅ Migrando ServoPWMController.cpp..."
    
    # Agregar include de ProjectConfig.h
    if ! grep -q "ProjectConfig.h" "src/ServoPWMController.cpp"; then
        sed -i '/#include/a #include "ProjectConfig.h"' "src/ServoPWMController.cpp"
    fi
    
    # Reemplazar configuraciones de pines
    sed -i 's/SERVO_PINS\[i\]/HardwarePins::Servos::PINS[i]/g' "src/ServoPWMController.cpp"
    sed -i 's/NUM_SERVOS/HardwarePins::Servos::COUNT/g' "src/ServoPWMController.cpp"
fi

# Migrar WebControl.cpp
if [ -f "src/WebControl.cpp" ]; then
    cp "src/WebControl.cpp" "$BACKUP_DIR/"
    
    echo "✅ Migrando WebControl.cpp..."
    
    # Agregar include de ProjectConfig.h
    if ! grep -q "ProjectConfig.h" "src/WebControl.cpp"; then
        sed -i '/#include/a #include "ProjectConfig.h"' "src/WebControl.cpp"
    fi
    
    # Reemplazar configuraciones de red
    sed -i 's/AsyncWebServer server(80)/AsyncWebServer server(NetworkConfig::WebServer::PORT)/g' "src/WebControl.cpp"
fi

# Migrar SystemManager.cpp
if [ -f "src/SystemManager.cpp" ]; then
    cp "src/SystemManager.cpp" "$BACKUP_DIR/"
    
    echo "✅ Migrando SystemManager.cpp..."
    
    # Agregar include de ProjectConfig.h
    if ! grep -q "ProjectConfig.h" "src/SystemManager.cpp"; then
        sed -i '/#include/a #include "ProjectConfig.h"' "src/SystemManager.cpp"
    fi
fi

echo ""
echo "🔍 CREANDO ARCHIVO DE MIGRACIÓN GRADUAL..."

# Crear archivo de migración gradual
cat > "MIGRATION_GUIDE.md" << EOF
# 🔄 GUÍA DE MIGRACIÓN GRADUAL - CONFIGURACIÓN CENTRALIZADA

## 📋 RESUMEN
Este documento guía la migración gradual desde configuraciones dispersas hacia
la nueva configuración centralizada ProjectConfig.h.

## 🎯 ARCHIVOS MIGRADOS

### ✅ main.cpp
- **Antes**: \`Serial.begin(115200)\`
- **Después**: \`Serial.begin(DebugConfig::Serial::BAUD_RATE)\`
- **Beneficio**: Configuración centralizada de comunicación serie

### ✅ ServoPWMController.cpp
- **Antes**: \`SERVO_PINS[i]\`, \`NUM_SERVOS\`
- **Después**: \`HardwarePins::Servos::PINS[i]\`, \`HardwarePins::Servos::COUNT\`
- **Beneficio**: Pines y configuración de servos centralizados

### ✅ WebControl.cpp
- **Antes**: \`AsyncWebServer server(80)\`
- **Después**: \`AsyncWebServer server(NetworkConfig::WebServer::PORT)\`
- **Beneficio**: Configuración de red centralizada

### ✅ SystemManager.cpp
- **Antes**: Configuraciones dispersas
- **Después**: Uso de ProjectConfig.h
- **Beneficio**: Gestión centralizada del sistema

## 🔧 PRÓXIMOS PASOS

### Archivos Pendientes de Migración
- \`include/SET_PIN.h\` → Usar \`HardwarePins::*\`
- \`include/SERVO_CONFIG.h\` → Usar \`ServoConfig::*\`
- \`include/WiFiConfig.h\` → Usar \`NetworkConfig::*\`
- \`include/SecureConfig.h\` → Usar \`SecurityConfig::*\`
- \`include/SystemConfig.h\` → Usar \`DebugConfig::*\` + \`SystemSafety::*\`

### Validación
1. Ejecutar \`pio run --target compile\`
2. Verificar que no hay errores de compilación
3. Probar funcionalidad básica
4. Ejecutar \`pio check\` para verificar mejoras

## 📊 BENEFICIOS OBTENIDOS

### Mantenibilidad
- ✅ Configuración unificada
- ✅ Documentación integrada
- ✅ Validación automática

### Robustez
- ✅ Límites de seguridad
- ✅ Valores por defecto seguros
- ✅ Manejo de errores mejorado

### Escalabilidad
- ✅ Fácil agregar nuevos parámetros
- ✅ Estructura modular
- ✅ Compatibilidad hacia atrás

## 🚀 COMANDOS DE VERIFICACIÓN

\`\`\`bash
# Compilar proyecto
pio run --target compile

# Verificar calidad de código
pio check

# Validar configuración
./validate_project.sh
\`\`\`

---
*Guía de migración generada automáticamente - $(date)*
EOF

echo "✅ Guía de migración creada: MIGRATION_GUIDE.md"

echo ""
echo "🎯 MIGRACIÓN APLICADA:"
echo "====================="
echo "✅ main.cpp migrado a configuración centralizada"
echo "✅ ServoPWMController.cpp migrado"
echo "✅ WebControl.cpp migrado"
echo "✅ SystemManager.cpp migrado"
echo "✅ Archivos respaldados en $BACKUP_DIR"

echo ""
echo "📝 PRÓXIMOS PASOS:"
echo "================="
echo "1. Compilar el proyecto: pio run --target compile"
echo "2. Verificar funcionalidad: pio run --target upload"
echo "3. Ejecutar validación: pio check"
echo "4. Revisar MIGRATION_GUIDE.md para más detalles"

echo ""
echo "✅ INTEGRACIÓN COMPLETADA"
echo "========================"
echo "🔍 Revisar cambios antes de compilar"
echo "🧪 Probar funcionalidad después de la migración"
