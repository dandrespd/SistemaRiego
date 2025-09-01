#ifndef __WEB_CONTROL_H__
#define __WEB_CONTROL_H__

#include <stdint.h>

// Forward declarations
class SystemManager;

/**
 * @file WebControl.h
 * @brief Interface para el control web del sistema de riego con mejoras de robustez.
 * 
 * **FASE 3: MEJORAS DE ROBUSTEZ Y CONFIABILIDAD**
 * - Sistema de logging estructurado
 * - Watchdog software para autorecovery
 * - Validación de configuración en runtime
 * - Mecanismos de retry para operaciones críticas
 * 
 * @author Sistema de Riego Inteligente
 * @version 3.3 - Sistema Robusto
 * @date 2025
 */

/**
 * @brief Configuración simplificada del sistema de control web
 * @param systemManager Referencia al gestor del sistema
 */
void setupWebControl(SystemManager* systemManager);

/**
 * @brief Función de watchdog para verificar y recuperar el sistema web
 * @return true si el sistema está saludable, false si necesita recuperación
 */
bool checkWebSystemHealth();

/**
 * @brief Reinicia el contador del watchdog
 */
void resetWebSystemWatchdog();

/**
 * @brief Intenta recuperar el sistema web después de una falla
 * @param systemManager Referencia al gestor del sistema
 * @return true si la recuperación fue exitosa
 */
bool recoverWebSystem(SystemManager* systemManager);

/**
 * @brief Obtiene el número de intentos de inicialización
 * @return Número de intentos realizados
 */
uint32_t getWebSystemInitializationAttempts();

/**
 * @brief Verifica si el sistema web está inicializado
 * @return true si el sistema está inicializado y funcionando
 */
bool isWebSystemInitialized();

/**
 * @brief Función principal de mantenimiento del sistema web
 * @param systemManager Referencia al gestor del sistema para posibles recuperaciones
 */
void maintainWebSystem(SystemManager* systemManager);

#endif // __WEB_CONTROL_H__
