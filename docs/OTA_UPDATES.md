# 🌐 Actualizaciones Over-The-Air (OTA) - Sistema de Riego Inteligente

## 📋 Introducción

El sistema de riego inteligente soporta actualizaciones Over-The-Air (OTA) que permiten actualizar el firmware remotamente sin necesidad de conexión física via USB. Esta guía explica cómo configurar y utilizar esta funcionalidad.

## 🚀 Configuración OTA

### Configuración en PlatformIO

El archivo `platformio.ini` está configurado para soportar OTA:

\`\`\`ini
upload_protocol = espota
upload_port = riego-inteligente.local
upload_flags =
    --auth=riego2025secure
    --port=3232
\`\`\`

### Parámetros de Configuración

- **Hostname**: `riego-inteligente.local` (accesible via mDNS)
- **Puerto OTA**: `3232` (puerto dedicado para actualizaciones)
- **Puerto Web**: `80` (interfaz web de control)
- **Autenticación**: Contraseña segura habilitada

## 🔧 Comandos de Actualización

### Actualización desde PlatformIO

\`\`\`bash
# Actualizar via OTA (requiere que el ESP32 esté en la misma red)
platformio run --target upload --upload-port riego-inteligente.local

# Alternativa con IP específica
platformio run --target upload --upload-port 192.168.1.100
\`\`\`

### Actualización Manual via Web

Acceda a la interfaz web del sistema y use la sección "Actualizaciones OTA" para cargar un nuevo firmware binario.

## ⚙️ Configuración de Red Requerida

### Para OTA Funcional

1. **mDNS habilitado**: El sistema usa mDNS para ser accesible como `riego-inteligente.local`
2. **Puertos abiertos**: Asegúrese que los puertos 3232 (OTA) y 80 (Web) estén accesibles
3. **Red estable**: Conexión WiFi confiable durante la actualización

### Solución de Problemas de Red

Si `riego-inteligente.local` no funciona:

1. **Usar IP directa**: Encuentre la IP del ESP32 via Serial Monitor
2. **Verificar mDNS**: Asegúrese que mDNS esté soportado en su red
3. **Firewall**: Verifique que no haya bloqueos de puertos

## 🛡️ Seguridad OTA

### Características de Seguridad

- **Autenticación requerida**: Contraseña `riego2025secure` (configurable)
- **Conexión cifrada**: Las actualizaciones usan conexiones seguras
- **Verificación de integridad**: Validación automática del firmware

### Cambiar Contraseña OTA

Para cambiar la contraseña predeterminada:

1. Edite `src/main.cpp` y modifique la línea:
   \`\`\`cpp
   ArduinoOTA.setPassword("nueva-contraseña-segura");
   \`\`\`
2. Actualice también `platformio.ini`:
   \`\`\`ini
   upload_flags = --auth=nueva-contraseña-segura
   \`\`\`

## ⚠️ Consideraciones Importantes

### Durante Actualizaciones

1. **No desconecte la alimentación**: Podría dañar el firmware
2. **Mantenga conexión estable**: Interrupciones pueden causar fallos
3. **Tiempo estimado**: 30-60 segundos dependiendo del tamaño del firmware

### Estados de Actualización

El sistema muestra diferentes estados durante la OTA:

- **0-10%**: Inicialización y verificación
- **10-90%**: Transferencia del firmware
- **90-100%**: Validación y escritura final
- **100%**: Reinicio automático

## 🔍 Monitoreo y Logs

### Logs de Serial Monitor

Durante una actualización OTA, el sistema muestra:

\`\`\`log
[OTA] Iniciando actualización de firmware...
[OTA] Progreso: 10%
[OTA] Progreso: 20%
...
[OTA] Actualización completada exitosamente!
\`\`\`

### Estados de Error

Los errores comunes incluyen:

- `OTA_AUTH_ERROR`: Contraseña incorrecta
- `OTA_BEGIN_ERROR`: Espacio insuficiente en flash
- `OTA_CONNECT_ERROR`: Problemas de red
- `OTA_RECEIVE_ERROR`: Datos corruptos durante transferencia

## 🛠️ Recuperación de Fallos

### Si la OTA Falló

1. **Reintentar**: Ejecute el comando de upload nuevamente
2. **Verificar red**: Asegúrese de tener conexión estable
3. **Serial recovery**: Conecte via USB y cargue firmware de respaldo

### Reset de Configuración OTA

Si tiene problemas persistentes:

1. Mantenga presionado el botón BOOT durante el encendido
2. El sistema entrará en modo de recuperación
3. Cargue firmware estable via USB serial

## 📈 Mejores Prácticas

### Para Desarrollo

1. **Testear localmente**: Verifique OTA en red local primero
2. **Versiones estables**: Use OTA solo con versiones validadas
3. **Backup configuraciones**: Exporte configuraciones antes de actualizar

### Para Producción

1. **Actualizaciones programadas**: Realice OTA en horarios de no uso
2. **Notificar usuarios**: Informe antes de actualizaciones críticas
3. **Plan de rollback**: Tenga preparado firmware anterior estable

## 🔗 Recursos Adicionales

### Documentación Oficial

- [PlatformIO OTA Documentation](https://docs.platformio.org/en/latest/platforms/espressif32.html#upload-using-ota)
- [ArduinoOTA Reference](https://github.com/esp8266/Arduino/blob/master/libraries/ArduinoOTA/README.md)

### Soporte Técnico

Para problemas específicos del sistema de riego:

1. Consulte los logs via Serial Monitor
2. Verifique la configuración de red
3. Contacte al equipo de soporte técnico

---

**Última actualización**: Agosto 2025  
**Versión OTA**: v3.2+  
**Soporte**: Compatible con ESP32 Dev Module
