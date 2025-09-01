"use client"

import { useState } from "react"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Badge } from "@/components/ui/badge"
import { Progress } from "@/components/ui/progress"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs"
import { Switch } from "@/components/ui/switch"
import { Separator } from "@/components/ui/separator"
import { Alert, AlertDescription } from "@/components/ui/alert"
import { IrrigationChart } from "@/components/irrigation-chart"
import { ZoneControlCard } from "@/components/zone-control-card"
import { ConnectionStatus } from "@/components/connection-status"
import { ScheduleConfig } from "@/components/schedule-config"
import { SystemConfig } from "@/components/system-config"
import { useIrrigationWebSocket } from "@/hooks/use-irrigation-websocket"
import {
  Droplets,
  Settings,
  Activity,
  Wifi,
  Clock,
  MemoryStick,
  Power,
  AlertTriangle,
  CheckCircle,
  XCircle,
  Play,
  Pause,
  RotateCcw,
  TrendingUp,
  BarChart3,
  Zap,
  Calendar,
} from "lucide-react"

export default function IrrigationDashboard() {
  const { isConnected, connectionState, systemStatus, logs, controlZone, updateConfig, requestLogs, reconnect } =
    useIrrigationWebSocket()

  const [activeTab, setActiveTab] = useState("dashboard")

  const formatUptime = (seconds: number) => {
    const hours = Math.floor(seconds / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)
    const secs = seconds % 60
    return `${hours}h ${minutes}m ${secs}s`
  }

  const formatMemory = (bytes: number) => {
    return `${Math.round(bytes / 1024)} KB`
  }

  const getZoneStatusColor = (state: string) => {
    switch (state) {
      case "OPEN":
        return "bg-primary text-primary-foreground"
      case "OPENING":
        return "bg-secondary text-secondary-foreground"
      case "CLOSING":
        return "bg-muted text-muted-foreground"
      default:
        return "bg-muted text-muted-foreground"
    }
  }

  const getZoneIcon = (state: string) => {
    switch (state) {
      case "OPEN":
        return <Droplets className="h-4 w-4" />
      case "OPENING":
        return <Play className="h-4 w-4" />
      case "CLOSING":
        return <Pause className="h-4 w-4" />
      default:
        return <XCircle className="h-4 w-4" />
    }
  }

  const memoryChartData = {
    labels: ["00:00", "04:00", "08:00", "12:00", "16:00", "20:00"],
    datasets: [
      {
        label: "Memoria Libre (KB)",
        data: [240, 245, 230, 250, 245, 248],
        borderColor: "hsl(var(--chart-1))",
        backgroundColor: "hsl(var(--chart-1) / 0.1)",
        tension: 0.4,
      },
    ],
  }

  const waterUsageData = {
    labels: ["Jardín", "Huerto", "Césped", "Macetas", "Invernadero"],
    datasets: [
      {
        label: "Consumo de Agua (L)",
        data: [45, 32, 28, 15, 22],
        backgroundColor: [
          "hsl(var(--chart-1))",
          "hsl(var(--chart-2))",
          "hsl(var(--chart-3))",
          "hsl(var(--chart-4))",
          "hsl(var(--chart-5))",
        ],
      },
    ],
  }

  const wifiSignalData = {
    labels: ["Lun", "Mar", "Mié", "Jue", "Vie", "Sáb", "Dom"],
    datasets: [
      {
        label: "Señal WiFi (dBm)",
        data: [-62, -65, -58, -70, -63, -61, -65],
        borderColor: "hsl(var(--chart-2))",
        backgroundColor: "hsl(var(--chart-2) / 0.1)",
        tension: 0.4,
      },
    ],
  }

  const handleZoneToggle = (zoneId: number, duration?: number) => {
    const zone = systemStatus.zones.find((z) => z.zone === zoneId)
    if (zone) {
      const action = zone.state === "OPEN" ? "close" : "open"
      controlZone(zoneId, action, duration)
    }
  }

  const handleScheduleUpdate = (zoneId: number, schedule: any) => {
    console.log(`Updating schedule for zone ${zoneId}:`, schedule)
    updateConfig({ [`zone_${zoneId}_schedule`]: schedule })
  }

  const handleSystemModeChange = (mode: "AUTO" | "MANUAL" | "STOPPED") => {
    updateConfig({ operationMode: mode })
  }

  const handleEmergencyStop = () => {
    // Stop all zones immediately
    systemStatus.zones.forEach((zone) => {
      if (zone.state === "OPEN") {
        controlZone(zone.zone, "close")
      }
    })
    updateConfig({ emergencyStop: true })
  }

  const handleSystemRestart = () => {
    updateConfig({ systemRestart: true })
  }

  return (
    <div className="min-h-screen bg-background">
      <header className="border-b bg-card">
        <div className="container mx-auto px-4 py-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="flex items-center gap-2">
                <Droplets className="h-8 w-8 text-primary" />
                <div>
                  <h1 className="text-2xl font-bold text-foreground">Sistema Riego Inteligente</h1>
                  <p className="text-sm text-muted-foreground">v3.2 - Control y Monitoreo</p>
                </div>
              </div>
            </div>

            <div className="flex items-center gap-4">
              <ConnectionStatus isConnected={isConnected} connectionState={connectionState} onReconnect={reconnect} />

              <div className="flex items-center gap-2 text-sm">
                <Wifi className="h-4 w-4" />
                <span className="text-foreground">{systemStatus.wifi.ssid}</span>
                <Badge variant={systemStatus.wifi.connected ? "default" : "destructive"}>
                  {systemStatus.wifi.rssi} dBm
                </Badge>
              </div>

              <div className="flex items-center gap-2 text-sm">
                <Clock className="h-4 w-4" />
                <span className="text-foreground">
                  {systemStatus.rtc_time ? new Date(systemStatus.rtc_time).toLocaleTimeString() : "--:--:--"}
                </span>
              </div>

              <div className="flex items-center gap-2 text-sm">
                <MemoryStick className="h-4 w-4" />
                <span className="text-foreground">{formatMemory(systemStatus.mem_free)}</span>
              </div>
            </div>
          </div>
        </div>
      </header>

      <main className="container mx-auto px-4 py-6">
        <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
          <TabsList className="grid w-full grid-cols-5">
            <TabsTrigger value="dashboard" className="flex items-center gap-2">
              <Activity className="h-4 w-4" />
              Dashboard
            </TabsTrigger>
            <TabsTrigger value="control" className="flex items-center gap-2">
              <Power className="h-4 w-4" />
              Control
            </TabsTrigger>
            <TabsTrigger value="config" className="flex items-center gap-2">
              <Settings className="h-4 w-4" />
              Configuración
            </TabsTrigger>
            <TabsTrigger value="monitor" className="flex items-center gap-2">
              <BarChart3 className="h-4 w-4" />
              Monitor
            </TabsTrigger>
            <TabsTrigger value="ota" className="flex items-center gap-2">
              <RotateCcw className="h-4 w-4" />
              OTA
            </TabsTrigger>
          </TabsList>

          <TabsContent value="dashboard" className="space-y-6">
            {!isConnected && (
              <Alert>
                <AlertTriangle className="h-4 w-4" />
                <AlertDescription>
                  Sin conexión con el sistema de riego. Los datos mostrados pueden no estar actualizados.
                  <Button variant="link" className="p-0 h-auto ml-2" onClick={reconnect}>
                    Reconectar
                  </Button>
                </AlertDescription>
              </Alert>
            )}

            <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-4">
              <Card>
                <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                  <CardTitle className="text-sm font-medium">Estado del Sistema</CardTitle>
                  <Activity className="h-4 w-4 text-muted-foreground" />
                </CardHeader>
                <CardContent>
                  <div className="text-2xl font-bold text-primary">{systemStatus.cycle}</div>
                  <p className="text-xs text-muted-foreground">Tiempo activo: {formatUptime(systemStatus.uptime_s)}</p>
                </CardContent>
              </Card>

              <Card>
                <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                  <CardTitle className="text-sm font-medium">Zonas Activas</CardTitle>
                  <Droplets className="h-4 w-4 text-muted-foreground" />
                </CardHeader>
                <CardContent>
                  <div className="text-2xl font-bold text-primary">
                    {systemStatus.zones.filter((z) => z.state === "OPEN").length}
                  </div>
                  <p className="text-xs text-muted-foreground">de {systemStatus.zones.length} zonas totales</p>
                </CardContent>
              </Card>

              <Card>
                <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                  <CardTitle className="text-sm font-medium">Memoria Libre</CardTitle>
                  <MemoryStick className="h-4 w-4 text-muted-foreground" />
                </CardHeader>
                <CardContent>
                  <div className="text-2xl font-bold text-primary">{formatMemory(systemStatus.mem_free)}</div>
                  <Progress value={75} className="mt-2" />
                </CardContent>
              </Card>

              <Card>
                <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                  <CardTitle className="text-sm font-medium">Consumo Hoy</CardTitle>
                  <TrendingUp className="h-4 w-4 text-muted-foreground" />
                </CardHeader>
                <CardContent>
                  <div className="text-2xl font-bold text-primary">142L</div>
                  <p className="text-xs text-muted-foreground">+12% vs ayer</p>
                </CardContent>
              </Card>
            </div>

            <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
              <Card className="col-span-full lg:col-span-2">
                <CardHeader>
                  <CardTitle>Uso de Memoria del Sistema</CardTitle>
                  <CardDescription>Monitoreo de memoria libre en las últimas 24 horas</CardDescription>
                </CardHeader>
                <CardContent>
                  <IrrigationChart type="line" data={memoryChartData} title="" height={250} />
                </CardContent>
              </Card>

              <Card>
                <CardHeader>
                  <CardTitle>Consumo por Zona</CardTitle>
                  <CardDescription>Litros de agua utilizados hoy</CardDescription>
                </CardHeader>
                <CardContent>
                  <IrrigationChart type="bar" data={waterUsageData} title="" height={250} />
                </CardContent>
              </Card>
            </div>

            <Card>
              <CardHeader>
                <CardTitle>Estado de las Zonas</CardTitle>
                <CardDescription>Control y monitoreo en tiempo real de todas las zonas de riego</CardDescription>
              </CardHeader>
              <CardContent>
                <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
                  {systemStatus.zones.map((zone) => (
                    <ZoneControlCard
                      key={zone.zone}
                      zone={zone}
                      onToggle={handleZoneToggle}
                      onScheduleUpdate={handleScheduleUpdate}
                    />
                  ))}
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Alertas del Sistema</CardTitle>
                <CardDescription>Notificaciones y estado de componentes críticos</CardDescription>
              </CardHeader>
              <CardContent className="space-y-3">
                <Alert>
                  <CheckCircle className="h-4 w-4" />
                  <AlertDescription>
                    {isConnected
                      ? "Sistema conectado y funcionando correctamente"
                      : "Sistema funcionando en modo local"}
                  </AlertDescription>
                </Alert>
                <Alert>
                  <Zap className="h-4 w-4" />
                  <AlertDescription>Señal WiFi estable: {systemStatus.wifi.rssi} dBm</AlertDescription>
                </Alert>
              </CardContent>
            </Card>
          </TabsContent>

          <TabsContent value="control" className="space-y-6">
            <div className="grid gap-6 md:grid-cols-2">
              <Card>
                <CardHeader>
                  <CardTitle>Control del Sistema</CardTitle>
                  <CardDescription>Configuración general del modo de operación</CardDescription>
                </CardHeader>
                <CardContent className="space-y-4">
                  <div className="flex items-center justify-between">
                    <div className="space-y-1">
                      <h4 className="font-medium">Modo Automático</h4>
                      <p className="text-sm text-muted-foreground">Riego programado según horarios</p>
                    </div>
                    <Switch
                      checked={systemStatus.cycle === "AUTO"}
                      onCheckedChange={(checked) => checked && handleSystemModeChange("AUTO")}
                    />
                  </div>
                  <Separator />
                  <div className="flex items-center justify-between">
                    <div className="space-y-1">
                      <h4 className="font-medium">Modo Manual</h4>
                      <p className="text-sm text-muted-foreground">Control directo de zonas</p>
                    </div>
                    <Switch
                      checked={systemStatus.cycle === "MANUAL"}
                      onCheckedChange={(checked) => checked && handleSystemModeChange("MANUAL")}
                    />
                  </div>
                  <Separator />
                  <Button variant="destructive" className="w-full" onClick={handleEmergencyStop}>
                    <Pause className="h-4 w-4 mr-2" />
                    Parada de Emergencia
                  </Button>
                </CardContent>
              </Card>

              <Card>
                <CardHeader>
                  <CardTitle>Acciones Rápidas</CardTitle>
                  <CardDescription>Controles de emergencia y mantenimiento</CardDescription>
                </CardHeader>
                <CardContent className="space-y-4">
                  <Button className="w-full bg-transparent" variant="outline">
                    <Play className="h-4 w-4 mr-2" />
                    Riego de Emergencia (5 min)
                  </Button>
                  <Button className="w-full bg-transparent" variant="outline" onClick={handleSystemRestart}>
                    <RotateCcw className="h-4 w-4 mr-2" />
                    Reiniciar Sistema
                  </Button>
                  <Button className="w-full bg-transparent" variant="outline">
                    <Settings className="h-4 w-4 mr-2" />
                    Calibrar Servos
                  </Button>
                </CardContent>
              </Card>
            </div>

            <Card>
              <CardHeader>
                <CardTitle>Control Individual de Zonas</CardTitle>
                <CardDescription>Gestión detallada de cada zona de riego</CardDescription>
              </CardHeader>
              <CardContent>
                <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
                  {systemStatus.zones.map((zone) => (
                    <ZoneControlCard
                      key={zone.zone}
                      zone={zone}
                      onToggle={handleZoneToggle}
                      onScheduleUpdate={handleScheduleUpdate}
                    />
                  ))}
                </div>
              </CardContent>
            </Card>
          </TabsContent>

          <TabsContent value="config" className="space-y-6">
            <Tabs defaultValue="system" className="w-full">
              <TabsList className="grid w-full grid-cols-2">
                <TabsTrigger value="system" className="flex items-center gap-2">
                  <Settings className="h-4 w-4" />
                  Sistema
                </TabsTrigger>
                <TabsTrigger value="schedule" className="flex items-center gap-2">
                  <Calendar className="h-4 w-4" />
                  Programación
                </TabsTrigger>
              </TabsList>

              <TabsContent value="system">
                <SystemConfig onConfigUpdate={updateConfig} />
              </TabsContent>

              <TabsContent value="schedule">
                <ScheduleConfig
                  zones={systemStatus.zones.map((z) => ({ zone: z.zone, name: z.name }))}
                  onScheduleUpdate={handleScheduleUpdate}
                />
              </TabsContent>
            </Tabs>
          </TabsContent>

          <TabsContent value="monitor" className="space-y-6">
            <div className="grid gap-6 md:grid-cols-2">
              <Card>
                <CardHeader>
                  <CardTitle>Señal WiFi</CardTitle>
                  <CardDescription>Historial de conectividad de la última semana</CardDescription>
                </CardHeader>
                <CardContent>
                  <IrrigationChart type="line" data={wifiSignalData} title="" height={200} />
                </CardContent>
              </Card>

              <Card>
                <CardHeader>
                  <CardTitle>Estadísticas del Sistema</CardTitle>
                  <CardDescription>Métricas de rendimiento en tiempo real</CardDescription>
                </CardHeader>
                <CardContent className="space-y-4">
                  <div className="flex justify-between">
                    <span className="text-sm">CPU Usage:</span>
                    <span className="text-sm font-mono">23%</span>
                  </div>
                  <Progress value={23} />

                  <div className="flex justify-between">
                    <span className="text-sm">Temperatura:</span>
                    <span className="text-sm font-mono">42°C</span>
                  </div>
                  <Progress value={60} />

                  <div className="flex justify-between">
                    <span className="text-sm">Uptime:</span>
                    <span className="text-sm font-mono">{formatUptime(systemStatus.uptime_s)}</span>
                  </div>
                </CardContent>
              </Card>
            </div>

            <Card>
              <CardHeader>
                <div className="flex items-center justify-between">
                  <div>
                    <CardTitle>Registro de Eventos</CardTitle>
                    <CardDescription>Log del sistema en tiempo real</CardDescription>
                  </div>
                  <Button variant="outline" size="sm" onClick={() => requestLogs(100)}>
                    Actualizar Logs
                  </Button>
                </div>
              </CardHeader>
              <CardContent>
                <div className="space-y-2 max-h-96 overflow-y-auto">
                  <div className="rounded-lg bg-muted p-4 font-mono text-sm space-y-2">
                    {logs.length > 0 ? (
                      logs.map((log, index) => (
                        <div key={index} className="flex items-center gap-2">
                          {log.level === "ERROR" ? (
                            <XCircle className="h-4 w-4 text-destructive flex-shrink-0" />
                          ) : log.level === "WARN" ? (
                            <AlertTriangle className="h-4 w-4 text-muted flex-shrink-0" />
                          ) : (
                            <CheckCircle className="h-4 w-4 text-primary flex-shrink-0" />
                          )}
                          <span className="text-xs text-muted-foreground">
                            [{new Date(log.ts).toLocaleTimeString()}]
                          </span>
                          <span>{log.msg}</span>
                        </div>
                      ))
                    ) : (
                      <>
                        <div className="flex items-center gap-2">
                          <CheckCircle className="h-4 w-4 text-primary flex-shrink-0" />
                          <span className="text-xs text-muted-foreground">[{new Date().toLocaleTimeString()}]</span>
                          <span>Sistema iniciado correctamente</span>
                        </div>
                        <div className="flex items-center gap-2">
                          <CheckCircle className="h-4 w-4 text-primary flex-shrink-0" />
                          <span className="text-xs text-muted-foreground">[{new Date().toLocaleTimeString()}]</span>
                          <span>WiFi conectado a {systemStatus.wifi.ssid}</span>
                        </div>
                        <div className="flex items-center gap-2">
                          <Droplets className="h-4 w-4 text-secondary flex-shrink-0" />
                          <span className="text-xs text-muted-foreground">[{new Date().toLocaleTimeString()}]</span>
                          <span>Sistema en modo {systemStatus.cycle}</span>
                        </div>
                        <div className="flex items-center gap-2">
                          <AlertTriangle className="h-4 w-4 text-muted flex-shrink-0" />
                          <span className="text-xs text-muted-foreground">[{new Date().toLocaleTimeString()}]</span>
                          <span>Memoria libre: {formatMemory(systemStatus.mem_free)}</span>
                        </div>
                      </>
                    )}
                  </div>
                </div>
              </CardContent>
            </Card>
          </TabsContent>

          <TabsContent value="ota" className="space-y-6">
            <Card>
              <CardHeader>
                <CardTitle>Actualización Over-The-Air</CardTitle>
                <CardDescription>Actualizar firmware del sistema remotamente</CardDescription>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="border-2 border-dashed border-muted rounded-lg p-8 text-center">
                  <RotateCcw className="h-12 w-12 text-muted-foreground mx-auto mb-4" />
                  <h3 className="font-medium mb-2">Seleccionar archivo de firmware</h3>
                  <p className="text-sm text-muted-foreground mb-4">
                    Arrastra un archivo .bin aquí o haz clic para seleccionar
                  </p>
                  <Button variant="outline">Seleccionar Archivo</Button>
                </div>

                <div className="space-y-2">
                  <div className="flex justify-between text-sm">
                    <span>Versión actual:</span>
                    <span className="font-mono">v3.2.0</span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span>Última actualización:</span>
                    <span>2025-01-15 14:30:00</span>
                  </div>
                </div>
              </CardContent>
            </Card>
          </TabsContent>
        </Tabs>
      </main>
    </div>
  )
}
