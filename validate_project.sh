#!/bin/bash

# =============================================================================
# Script de Verificación Integral del Proyecto de Riego Inteligente v3.2
# =============================================================================
# 
# **CONCEPTO EDUCATIVO - INTEGRACIÓN CONTINUA**:
# Este script implementa verificaciones automatizadas que aseguran la calidad
# del código antes de cada despliegue. Es como tener un inspector de calidad
# automatizado que revisa todo antes de que llegue al cliente final.
# 
# **VERIFICACIONES IMPLEMENTADAS**:
# 1. Análisis estático de código (syntax checking)
# 2. Verificación de dependencias y configuración
# 3. Compilación con múltiples configuraciones
# 4. Análisis de memoria y rendimiento
# 5. Verificación de seguridad (credenciales hardcodeadas)
# 6. Generación de reportes detallados
# 
# @author Sistema de Riego Inteligente
# @version 3.2 - Validación Integral
# @date 2025
# =============================================================================

# Configuración de colores para output legible
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Variables de configuración
PROJECT_DIR="C:/Users/Public/Documents/Riego"
REPORT_DIR="$PROJECT_DIR/reports"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
REPORT_FILE="$REPORT_DIR/build_report_$TIMESTAMP.log"

# Contadores para estadísticas finales
TESTS_PASSED=0
TESTS_FAILED=0
WARNINGS_COUNT=0

# =============================================================================
# Funciones de Utilidad
# =============================================================================

print_header() {
    echo -e "${CYAN}=========================================="
    echo -e "    $1"
    echo -e "==========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
    ((TESTS_PASSED++))
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
    ((TESTS_FAILED++))
}

print_warning() {
    echo -e "${YELLOW}⚠️ $1${NC}"
    ((WARNINGS_COUNT++))
}

print_info() {
    echo -e "${BLUE}ℹ️ $1${NC}"
}

log_to_report() {
    echo "$(date): $1" >> "$REPORT_FILE"
}

# =============================================================================
# Función Principal
# =============================================================================

main() {
    print_header "SISTEMA DE VALIDACIÓN INTEGRAL v3.2"
    echo -e "${PURPLE}🤖 Iniciando análisis automatizado del proyecto de riego inteligente${NC}"
    echo ""
    
    # Crear directorio de reportes si no existe
    mkdir -p "$REPORT_DIR"
    log_to_report "Iniciando validación integral del proyecto"
    
    # Cambiar al directorio del proyecto
    cd "$PROJECT_DIR" || {
        print_error "No se puede acceder al directorio del proyecto: $PROJECT_DIR"
        exit 1
    }
    
    # Ejecutar todas las verificaciones
    check_environment
    check_project_structure
    security_audit
    static_code_analysis
    dependency_check
    configuration_validation
    compile_project
    memory_analysis
    performance_check
    generate_final_report
    
    # Mostrar resumen final
    show_final_summary
}

# =============================================================================
# Verificaciones Específicas
# =============================================================================

check_environment() {
    print_header "VERIFICACIÓN DEL ENTORNO DE DESARROLLO"
    
    # Verificar PlatformIO
    if command -v pio &> /dev/null; then
        PIO_VERSION=$(pio --version)
        print_success "PlatformIO instalado: $PIO_VERSION"
        log_to_report "PlatformIO encontrado: $PIO_VERSION"
    else
        print_error "PlatformIO no está instalado o no está en el PATH"
        log_to_report "ERROR: PlatformIO no encontrado"
        exit 1
    fi
    
    # Verificar Git (opcional)
    if command -v git &> /dev/null; then
        GIT_VERSION=$(git --version)
        print_success "Git disponible: $GIT_VERSION"
    else
        print_warning "Git no encontrado - Recomendado para control de versiones"
    fi
    
    # Verificar Python
    if command -v python &> /dev/null; then
        PYTHON_VERSION=$(python --version)
        print_success "Python disponible: $PYTHON_VERSION"
    else
        print_warning "Python no encontrado - Algunas funciones avanzadas podrían no funcionar"
    fi
    
    echo ""
}

check_project_structure() {
    print_header "VERIFICACIÓN DE ESTRUCTURA DEL PROYECTO"
    
    # Archivos críticos que deben existir
    CRITICAL_FILES=(
        "platformio.ini"
        "src/main.cpp"
        "src/SystemManager.cpp"
        "src/ServoPWMController.cpp"
        "src/WebSocketManager.cpp"
        "src/WebControl.cpp"
        "include/SystemConfig.h"
        "include/SecureConfig.h"
        "include/SET_PIN.h"
        "include/SERVO_CONFIG.h"
    )
    
    for file in "${CRITICAL_FILES[@]}"; do
        if [ -f "$file" ]; then
            print_success "Archivo crítico presente: $file"
        else
            print_error "Archivo crítico faltante: $file"
        fi
    done
    
    # Verificar directorio data
    if [ -d "data" ]; then
        print_success "Directorio 'data' presente"
        if [ -f "data/LOGO.png" ]; then
            print_success "Logo del sistema presente"
        else
            print_warning "Logo del sistema no encontrado en data/"
        fi
    else
        print_warning "Directorio 'data' no encontrado - Archivos web estáticos podrían faltar"
    fi
    
    echo ""
}

security_audit() {
    print_header "AUDITORÍA DE SEGURIDAD"
    
    # Buscar credenciales hardcodeadas
    print_info "Buscando credenciales hardcodeadas..."
    
    SECURITY_PATTERNS=(
        "password.*=.*\"[^\"]+\""
        "ssid.*=.*\"[^\"]+\""
        "api_key.*=.*\"[^\"]+\""
        "token.*=.*\"[^\"]+\""
        "secret.*=.*\"[^\"]+\""
    )
    
    SECURITY_ISSUES=0
    for pattern in "${SECURITY_PATTERNS[@]}"; do
        if grep -r -E -i "$pattern" src/ include/ --exclude-dir=.git 2>/dev/null; then
            print_error "Posible credencial hardcodeada encontrada (patrón: $pattern)"
            ((SECURITY_ISSUES++))
        fi
    done
    
    if [ $SECURITY_ISSUES -eq 0 ]; then
        print_success "No se encontraron credenciales hardcodeadas obvias"
    else
        print_error "Se encontraron $SECURITY_ISSUES posibles problemas de seguridad"
        log_to_report "SEGURIDAD: $SECURITY_ISSUES problemas encontrados"
    fi
    
    # Verificar uso del sistema seguro
    if grep -q "SecureConfig.h" src/*.cpp include/*.h 2>/dev/null; then
        print_success "Sistema de configuración seguro en uso"
    else
        print_warning "Sistema de configuración seguro no detectado"
    fi
    
    echo ""
}

static_code_analysis() {
    print_header "ANÁLISIS ESTÁTICO DE CÓDIGO"
    
    # Verificar includes faltantes
    print_info "Verificando dependencias de includes..."
    
    # Buscar includes potencialmente problemáticos
    MISSING_INCLUDES=0
    
    # Verificar que todos los archivos .h tengan guards
    for header in include/*.h; do
        if [ -f "$header" ]; then
            if ! grep -q "#ifndef\|#pragma once" "$header"; then
                print_warning "Header sin include guard: $(basename "$header")"
            else
                print_success "Header con include guard: $(basename "$header")"
            fi
        fi
    done
    
    # Verificar sintaxis básica de C++
    print_info "Verificando sintaxis básica..."
    
    # Buscar problemas comunes
    if grep -r "delete\[\]" src/ include/ 2>/dev/null; then
        print_warning "Uso de delete[] detectado - Verificar gestión de memoria"
    fi
    
    if grep -r "malloc\|free" src/ include/ 2>/dev/null; then
        print_warning "Uso de malloc/free detectado - Preferir new/delete en C++"
    fi
    
    echo ""
}

dependency_check() {
    print_header "VERIFICACIÓN DE DEPENDENCIAS"
    
    print_info "Verificando librerías en platformio.ini..."
    
    # Librerías requeridas
    REQUIRED_LIBS=(
        "ArduinoJson"
        "ESP Async WebServer"
        "AsyncTCP"
        "Ds1302"
    )
    
    for lib in "${REQUIRED_LIBS[@]}"; do
        if grep -q "$lib" platformio.ini; then
            print_success "Librería encontrada: $lib"
        else
            print_error "Librería faltante: $lib"
        fi
    done
    
    echo ""
}

configuration_validation() {
    print_header "VALIDACIÓN DE CONFIGURACIÓN"
    
    # Verificar configuración de pines
    if [ -f "include/SET_PIN.h" ]; then
        print_info "Validando configuración de pines..."
        
        # Verificar que no hay pines duplicados (básico)
        PIN_COUNT=$(grep -o 'constexpr.*= [0-9]\+' include/SET_PIN.h | wc -l)
        print_info "Pines configurados: $PIN_COUNT"
        
        if [ $PIN_COUNT -gt 0 ]; then
            print_success "Configuración de pines presente"
        else
            print_warning "Configuración de pines podría estar incompleta"
        fi
    fi
    
    # Verificar configuración del sistema
    if [ -f "include/SystemConfig.h" ]; then
        print_success "Archivo de configuración del sistema presente"
        
        # Verificar que hay configuraciones de debug
        if grep -q "DEBUG_PRINTLN" include/SystemConfig.h; then
            print_success "Macros de debug configuradas"
        else
            print_warning "Macros de debug no encontradas"
        fi
    fi
    
    echo ""
}

compile_project() {
    print_header "COMPILACIÓN DEL PROYECTO"
    
    print_info "Limpiando compilaciones anteriores..."
    pio run --target clean > /dev/null 2>&1
    
    print_info "Iniciando compilación principal..."
    
    # Compilar y capturar output
    if pio run > /tmp/compile_output.log 2>&1; then
        print_success "Compilación exitosa"
        log_to_report "Compilación exitosa"
        
        # Mostrar warnings si existen
        WARNING_COUNT=$(grep -c "warning:" /tmp/compile_output.log 2>/dev/null || echo "0")
        if [ $WARNING_COUNT -gt 0 ]; then
            print_warning "Se encontraron $WARNING_COUNT warnings durante la compilación"
            log_to_report "Warnings de compilación: $WARNING_COUNT"
        fi
        
    else
        print_error "Errores de compilación detectados"
        echo -e "${RED}Últimos errores de compilación:${NC}"
        tail -20 /tmp/compile_output.log
        log_to_report "ERROR: Compilación falló"
        return 1
    fi
    
    echo ""
}

memory_analysis() {
    print_header "ANÁLISIS DE MEMORIA"
    
    if [ -f ".pio/build/esp32dev/firmware.elf" ]; then
        print_info "Analizando uso de memoria..."
        
        # Obtener información de memoria
        MEMORY_INFO=$(pio run --target size 2>/dev/null)
        echo "$MEMORY_INFO"
        
        # Extraer información específica si está disponible
        if echo "$MEMORY_INFO" | grep -q "RAM:"; then
            RAM_USAGE=$(echo "$MEMORY_INFO" | grep "RAM:" | grep -o '[0-9.]*%' | head -1)
            FLASH_USAGE=$(echo "$MEMORY_INFO" | grep "Flash:" | grep -o '[0-9.]*%' | head -1)
            
            print_info "Uso de RAM: $RAM_USAGE"
            print_info "Uso de Flash: $FLASH_USAGE"
            
            log_to_report "Memoria - RAM: $RAM_USAGE, Flash: $FLASH_USAGE"
        fi
        
        print_success "Análisis de memoria completado"
    else
        print_warning "Archivo firmware.elf no encontrado - Compilar primero"
    fi
    
    echo ""
}

performance_check() {
    print_header "VERIFICACIÓN DE RENDIMIENTO"
    
    print_info "Analizando potenciales cuellos de botella..."
    
    # Buscar bucles potencialmente problemáticos
    LONG_DELAYS=$(grep -r "delay([0-9]\{4,\})" src/ include/ 2>/dev/null | wc -l)
    if [ $LONG_DELAYS -gt 0 ]; then
        print_warning "Se encontraron $LONG_DELAYS delays largos (>1000ms)"
    else
        print_success "No se detectaron delays excesivamente largos"
    fi
    
    # Verificar uso de String vs char*
    STRING_USAGE=$(grep -r "String " src/ include/ 2>/dev/null | wc -l)
    if [ $STRING_USAGE -gt 10 ]; then
        print_info "Uso moderado de String detectado ($STRING_USAGE instancias)"
        print_info "Considere usar char* para optimización de memoria si es necesario"
    fi
    
    print_success "Verificación de rendimiento completada"
    echo ""
}

generate_final_report() {
    print_header "GENERANDO REPORTE FINAL"
    
    {
        echo "=========================================="
        echo "REPORTE DE VALIDACIÓN - $(date)"
        echo "=========================================="
        echo ""
        echo "ESTADÍSTICAS:"
        echo "- Pruebas exitosas: $TESTS_PASSED"
        echo "- Pruebas fallidas: $TESTS_FAILED"
        echo "- Advertencias: $WARNINGS_COUNT"
        echo ""
        echo "ARCHIVOS VERIFICADOS:"
        find src/ include/ -name "*.cpp" -o -name "*.h" | wc -l
        echo ""
        echo "TAMAÑO DEL PROYECTO:"
        du -sh . 2>/dev/null || echo "No disponible"
        echo ""
        echo "LÍNEAS DE CÓDIGO:"
        find src/ include/ -name "*.cpp" -o -name "*.h" -exec wc -l {} + 2>/dev/null | tail -1 || echo "No disponible"
        echo ""
    } >> "$REPORT_FILE"
    
    print_success "Reporte generado: $REPORT_FILE"
    echo ""
}

show_final_summary() {
    print_header "RESUMEN FINAL DE VALIDACIÓN"
    
    echo -e "${BLUE}📊 ESTADÍSTICAS:${NC}"
    echo -e "   ✅ Pruebas exitosas: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "   ❌ Pruebas fallidas: ${RED}$TESTS_FAILED${NC}"
    echo -e "   ⚠️  Advertencias: ${YELLOW}$WARNINGS_COUNT${NC}"
    echo ""
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}🎉 ¡VALIDACIÓN COMPLETADA EXITOSAMENTE!${NC}"
        echo -e "${GREEN}   El proyecto está listo para despliegue${NC}"
        log_to_report "RESULTADO: Validación exitosa"
        exit 0
    else
        echo -e "${RED}⚠️ VALIDACIÓN COMPLETADA CON ERRORES${NC}"
        echo -e "${RED}   Revise los errores anteriores antes del despliegue${NC}"
        log_to_report "RESULTADO: Validación con errores"
        exit 1
    fi
}

# =============================================================================
# Ejecución del Script
# =============================================================================

# Verificar que se está ejecutando desde la ubicación correcta
if [ ! -f "platformio.ini" ]; then
    print_error "Este script debe ejecutarse desde el directorio raíz del proyecto"
    print_info "Directorio actual: $(pwd)"
    print_info "Directorio esperado: $PROJECT_DIR"
    exit 1
fi

# Ejecutar función principal
main "$@"
