"use client"

import { useState, useEffect } from "react"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Button } from "@/components/ui/button"
import { ConnectionStatus } from "@/components/connection-status"
import { RTCConfig } from "@/components/rtc-config"
import { ZoneTableConfig } from "@/components/zone-table-config"
import { SystemConfig } from "@/components/system-config"
import { Settings, Clock, Droplets, Wifi, RefreshCw } from "lucide-react"
import { useIrrigationWebSocket } from "@/hooks/use-irrigation-websocket"
import { apiClient } from "@/lib/api"
import { useToast } from "@/hooks/use-toast"

interface RTCDateTime {
  year: number
  month: number
  day: number
  dayOfWeek: number
  hour: number
  minute: number
  second: number
}

interface ZoneConfig {
  id: number
  name: string
  gpio: number
  wateringTime: number
  enabled: boolean
  servoOpenAngle: number
  servoCloseAngle: number
}

export default function ConfigPage() {
  const { toast } = useToast()
  const { connectionStatus, systemData } = useIrrigationWebSocket()
  const [isLoading, setIsLoading] = useState(false)
  const [currentRTC, setCurrentRTC] = useState<RTCDateTime | undefined>()
  const [currentZones, setCurrentZones] = useState<ZoneConfig[] | undefined>()

  // Load current configuration on mount
  useEffect(() => {
    loadCurrentConfig()
  }, [])

  const loadCurrentConfig = async () => {
    setIsLoading(true)
    try {
      const [config, zoneConfig] = await Promise.all([apiClient.getConfig(), apiClient.getZoneConfig()])

      if (config.rtc) {
        setCurrentRTC(config.rtc)
      }
      if (zoneConfig.zones) {
        setCurrentZones(zoneConfig.zones)
      }
    } catch (error) {
      console.error("Error loading config:", error)
    } finally {
      setIsLoading(false)
    }
  }

  const handleRTCSave = async (rtcData: RTCDateTime): Promise<boolean> => {
    try {
      const result = await apiClient.setRTCDateTime(rtcData)
      if (result.status === "success") {
        setCurrentRTC(rtcData)
        // Send restart command
        await apiClient.restartRTC()
        return true
      }
      return false
    } catch (error) {
      console.error("Error saving RTC:", error)
      return false
    }
  }

  const handleZonesSave = async (zones: ZoneConfig[]): Promise<boolean> => {
    try {
      const result = await apiClient.updateZones(zones)
      if (result.status === "success") {
        setCurrentZones(zones)
        return true
      }
      return false
    } catch (error) {
      console.error("Error saving zones:", error)
      return false
    }
  }

  const handleSystemConfigSave = async (config: any): Promise<boolean> => {
    try {
      const result = await apiClient.updateSystemConfig(config)
      return result.status === "success"
    } catch (error) {
      console.error("Error saving system config:", error)
      return false
    }
  }

  return (
    <div className="container mx-auto p-6 space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Configuración del Sistema</h1>
          <p className="text-muted-foreground">Sistema de Riego Inteligente v3.2 - Configuración completa</p>
        </div>
        <div className="flex items-center gap-4">
          <ConnectionStatus status={connectionStatus} />
          <Button variant="outline" onClick={loadCurrentConfig} disabled={isLoading}>
            <RefreshCw className={`h-4 w-4 mr-2 ${isLoading ? "animate-spin" : ""}`} />
            Actualizar
          </Button>
        </div>
      </div>

      {/* System Status */}
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Settings className="h-5 w-5" />
            Estado del Sistema
          </CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-primary/10 rounded-lg">
                <Wifi className="h-4 w-4 text-primary" />
              </div>
              <div>
                <p className="text-sm font-medium">Conexión</p>
                <Badge variant={connectionStatus === "connected" ? "default" : "destructive"}>
                  {connectionStatus === "connected" ? "Conectado" : "Desconectado"}
                </Badge>
              </div>
            </div>

            <div className="flex items-center gap-3">
              <div className="p-2 bg-secondary/10 rounded-lg">
                <Clock className="h-4 w-4 text-secondary-foreground" />
              </div>
              <div>
                <p className="text-sm font-medium">RTC</p>
                <p className="text-xs text-muted-foreground">{systemData?.rtc_status || "No disponible"}</p>
              </div>
            </div>

            <div className="flex items-center gap-3">
              <div className="p-2 bg-green-100 rounded-lg">
                <Droplets className="h-4 w-4 text-green-600" />
              </div>
              <div>
                <p className="text-sm font-medium">Zonas Activas</p>
                <p className="text-xs text-muted-foreground">
                  {systemData?.zones?.filter((z) => z.state === "OPEN").length || 0} de 5
                </p>
              </div>
            </div>

            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-100 rounded-lg">
                <Settings className="h-4 w-4 text-blue-600" />
              </div>
              <div>
                <p className="text-sm font-medium">Modo</p>
                <p className="text-xs text-muted-foreground">{systemData?.operation_mode || "AUTO"}</p>
              </div>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Configuration Tabs */}
      <Tabs defaultValue="rtc" className="space-y-6">
        <TabsList className="grid w-full grid-cols-3">
          <TabsTrigger value="rtc" className="flex items-center gap-2">
            <Clock className="h-4 w-4" />
            RTC y Fecha/Hora
          </TabsTrigger>
          <TabsTrigger value="zones" className="flex items-center gap-2">
            <Droplets className="h-4 w-4" />
            Zonas de Riego
          </TabsTrigger>
          <TabsTrigger value="system" className="flex items-center gap-2">
            <Settings className="h-4 w-4" />
            Sistema
          </TabsTrigger>
        </TabsList>

        <TabsContent value="rtc" className="space-y-6">
          <RTCConfig onSave={handleRTCSave} currentDateTime={currentRTC} />
        </TabsContent>

        <TabsContent value="zones" className="space-y-6">
          <ZoneTableConfig onSave={handleZonesSave} initialZones={currentZones} />
        </TabsContent>

        <TabsContent value="system" className="space-y-6">
          <SystemConfig onSave={handleSystemConfigSave} />
        </TabsContent>
      </Tabs>
    </div>
  )
}
