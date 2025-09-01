# Sistema de Riego Inteligente v3.2 - Interfaz Web

Una interfaz web moderna y responsiva para el Sistema de Riego Inteligente basado en ESP32.

## 🌟 Características

- **Dashboard en tiempo real** con monitoreo de zonas de riego
- **Control WebSocket** para comunicación bidireccional
- **API REST** como respaldo cuando WebSocket no está disponible
- **Configuración dinámica** del sistema y horarios
- **Interfaz responsive** optimizada para móviles y escritorio
- **Tema oscuro/claro** con persistencia de preferencias
- **PWA ready** para instalación como aplicación nativa

## 🚀 Inicio Rápido

### Prerrequisitos

- Node.js 18+ y npm 8+
- ESP32 con el firmware del Sistema de Riego Inteligente v3.2
- Red WiFi compartida entre el ESP32 y el dispositivo de desarrollo

### Instalación

\`\`\`bash
# Clonar el repositorio
git clone https://github.com/your-username/riego-inteligente-web.git
cd riego-inteligente-web

# Instalar dependencias
npm install

# Configurar variables de entorno
cp .env.example .env.local
# Editar .env.local con la IP de tu ESP32
\`\`\`

### Desarrollo

\`\`\`bash
# Iniciar servidor de desarrollo
npm run dev

# Iniciar con acceso desde red local (para pruebas con ESP32)
npm run dev:esp32

# Probar conectividad con ESP32
npm run test:api
\`\`\`

### Producción

\`\`\`bash
# Construir para producción
npm run build:production

# Desplegar al ESP32 (experimental)
npm run deploy:esp32
\`\`\`

## 🔧 Configuración

### Variables de Entorno

\`\`\`env
# .env.local
NEXT_PUBLIC_WS_URL=ws://192.168.1.100/ws
ESP32_HOST=192.168.1.100
ESP32_PORT=80
ESP32_HTTPS=false
\`\`\`

### Configuración del ESP32

Asegúrate de que tu ESP32 tenga:
- Firmware del Sistema de Riego Inteligente v3.2
- WebSocket habilitado en `/ws`
- API REST disponible en `/api/*`
- SPIFFS configurado para archivos estáticos

## 📡 API y Comunicación

### WebSocket (Tiempo Real)
- **Endpoint**: `ws://[ESP32_IP]/ws`
- **Mensajes**: `state_update`, `log_entry`, `rtc_time`, `wifi_status`
- **Control**: Comandos de zona, configuración, logs

### REST API (Respaldo)
- **GET** `/api/status` - Estado del sistema
- **POST** `/api/config/rtc` - Configurar fecha/hora
- **GET/PUT** `/api/config` - Gestión de configuración
- **POST** `/api/config/reset` - Resetear configuración
- **GET/POST** `/api/config/backup` - Gestión de respaldos

## 🛠️ Scripts de Desarrollo

\`\`\`bash
# Probar conectividad con ESP32
ESP32_HOST=192.168.1.50 npm run test:api

# Desarrollo con acceso de red
npm run dev:esp32

# Construcción optimizada
npm run build:production

# Despliegue experimental al ESP32
ESP32_HOST=192.168.1.50 npm run deploy:esp32
\`\`\`

## 🏗️ Arquitectura

\`\`\`
src/
├── app/                 # Next.js App Router
├── components/          # Componentes React reutilizables
├── hooks/              # Custom hooks (WebSocket, API)
├── lib/                # Utilidades (API client, WebSocket)
└── scripts/            # Scripts de desarrollo y despliegue
\`\`\`

## 🎨 Tecnologías

- **Frontend**: Next.js 14, React 19, TypeScript
- **UI**: Tailwind CSS, Radix UI, Lucide Icons
- **Comunicación**: WebSocket nativo, Fetch API
- **Charts**: Recharts para visualización de datos
- **Desarrollo**: Scripts personalizados para ESP32

## 🤝 Contribuir

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## 📄 Licencia

Este proyecto está bajo la Licencia MIT. Ver `LICENSE` para más detalles.

## 🆘 Soporte

- **Issues**: [GitHub Issues](https://github.com/your-username/riego-inteligente-web/issues)
- **Documentación**: [Wiki del proyecto](https://github.com/your-username/riego-inteligente-web/wiki)
- **ESP32 Firmware**: Consulta la documentación del hardware

---

**Sistema de Riego Inteligente v3.2** - Automatización inteligente para tu jardín 🌱
