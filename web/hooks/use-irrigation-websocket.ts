"use client"

import { useEffect, useState, useCallback, useRef } from "react"
import {
  getWebSocketInstance,
  type StateUpdateMessage,
  type LogEntryMessage,
  type RTCTimeMessage,
  type WiFiStatusMessage,
} from "@/lib/websocket"
import { irrigationAPI, type IrrigationStatus } from "@/lib/api"

export interface SystemStatus {
  system: string
  uptime_s: number
  mem_free: number
  zones: Array<{
    zone: number
    name: string
    state: "OPEN" | "CLOSED" | "OPENING" | "CLOSING"
    remaining_s: number
    last_watered?: string
  }>
  wifi: {
    ssid: string
    rssi: number
    connected: boolean
  }
  rtc_time?: string
  cycle: "MANUAL" | "AUTO" | "STOPPED"
}

export interface LogEntry {
  level: "INFO" | "WARN" | "ERROR"
  msg: string
  ts: string
}

export function useIrrigationWebSocket() {
  const [isConnected, setIsConnected] = useState(false)
  const [connectionState, setConnectionState] = useState<"connecting" | "open" | "closing" | "closed">("closed")
  const [apiStatus, setApiStatus] = useState<IrrigationStatus | null>(null)
  const [lastApiUpdate, setLastApiUpdate] = useState<number>(0)
  const [systemStatus, setSystemStatus] = useState<SystemStatus>({
    system: "Riego Inteligente v3.2",
    uptime_s: 0,
    mem_free: 245760,
    zones: [
      { zone: 1, name: "Jardín Principal", state: "CLOSED", remaining_s: 0, last_watered: "2025-01-20T08:30:00Z" },
      { zone: 2, name: "Huerto", state: "CLOSED", remaining_s: 0, last_watered: "2025-01-20T07:15:00Z" },
      { zone: 3, name: "Césped", state: "CLOSED", remaining_s: 0, last_watered: "2025-01-19T18:45:00Z" },
      { zone: 4, name: "Macetas", state: "CLOSED", remaining_s: 0, last_watered: "2025-01-20T06:00:00Z" },
      { zone: 5, name: "Invernadero", state: "CLOSED", remaining_s: 0, last_watered: "2025-01-19T20:30:00Z" },
    ],
    wifi: { ssid: "MiRed_WiFi", rssi: -65, connected: true },
    rtc_time: new Date().toISOString(),
    cycle: "AUTO",
  })
  const [logs, setLogs] = useState<LogEntry[]>([])
  const wsRef = useRef(getWebSocketInstance())

  const fetchStatusViaAPI = useCallback(async () => {
    try {
      const status = await irrigationAPI.getStatus()
      setApiStatus(status)
      setLastApiUpdate(Date.now())

      setSystemStatus((prev) => ({
        ...prev,
        mem_free: status.memoryFree,
        zones: prev.zones.map((zone) =>
          zone.zone === status.activeZone
            ? { ...zone, state: "OPEN" as const, remaining_s: status.remainingTime }
            : { ...zone, state: "CLOSED" as const, remaining_s: 0 },
        ),
      }))

      console.log("[v0] API status updated:", status)
    } catch (error) {
      if (error instanceof Error && error.message === "ESP32_NOT_REACHABLE") {
        console.log("[v0] ESP32 not reachable, continuing in demo mode")
        return
      }
      console.error("[v0] Failed to fetch status via API:", error)
    }
  }, [])

  useEffect(() => {
    const ws = wsRef.current

    ws.onConnectionStateChange((connected) => {
      console.log("[v0] WebSocket connection state changed:", connected)
      setIsConnected(connected)
      setConnectionState(ws.getConnectionState())

      if (!connected) {
        console.log("[v0] WebSocket disconnected, starting API fallback")
        fetchStatusViaAPI()
      }
    })

    ws.onMessage<StateUpdateMessage>("state_update", (message) => {
      console.log("[v0] Processing state update:", message)
      setSystemStatus((prev) => ({
        ...prev,
        zones: message.zones.map((zone) => ({
          ...zone,
          last_watered: prev.zones.find((z) => z.zone === zone.zone)?.last_watered,
        })),
        cycle: message.cycle,
        mem_free: message.mem_free,
      }))
    })

    ws.onMessage<LogEntryMessage>("log_entry", (message) => {
      console.log("[v0] Processing log entry:", message)
      const newLog: LogEntry = {
        level: message.level,
        msg: message.msg,
        ts: message.ts,
      }
      setLogs((prev) => [newLog, ...prev.slice(0, 99)]) // Keep last 100 logs
    })

    ws.onMessage<RTCTimeMessage>("rtc_time", (message) => {
      setSystemStatus((prev) => ({
        ...prev,
        rtc_time: message.iso,
      }))
    })

    ws.onMessage<WiFiStatusMessage>("wifi_status", (message) => {
      console.log("[v0] Processing WiFi status:", message)
      setSystemStatus((prev) => ({
        ...prev,
        wifi: {
          ssid: message.ssid,
          rssi: message.rssi,
          connected: message.connected,
        },
      }))
    })

    ws.connect().catch((error) => {
      console.log("[v0] WebSocket connection failed, continuing in demo mode:", error)
      // Don't call fetchStatusViaAPI here to avoid double API calls
    })

    return () => {
      ws.disconnect()
    }
  }, [fetchStatusViaAPI])

  useEffect(() => {
    const interval = setInterval(() => {
      const ws = wsRef.current
      setConnectionState(ws.getConnectionState())

      if (ws.isConnected()) {
        setSystemStatus((prev) => ({
          ...prev,
          uptime_s: prev.uptime_s + 1,
          rtc_time: new Date().toISOString(),
        }))
      } else {
        const timeSinceLastUpdate = Date.now() - lastApiUpdate
        if (timeSinceLastUpdate > 5000) {
          fetchStatusViaAPI()
        }

        setSystemStatus((prev) => ({
          ...prev,
          uptime_s: prev.uptime_s + 1,
          rtc_time: new Date().toISOString(),
        }))
      }
    }, 1000)

    return () => clearInterval(interval)
  }, [lastApiUpdate, fetchStatusViaAPI])

  const controlZone = useCallback(
    async (zone: number, action: "open" | "close", duration_s?: number) => {
      const ws = wsRef.current
      console.log("[v0] Controlling zone:", zone, action, duration_s)

      if (ws.isConnected()) {
        ws.sendControlZone(zone, action, duration_s)
      } else {
        try {
          if (action === "open") {
            await irrigationAPI.startZone(zone, duration_s)
          } else {
            await irrigationAPI.stopZone(zone)
          }

          setTimeout(() => fetchStatusViaAPI(), 1000)

          console.log("[v0] Zone controlled via API:", zone, action)
        } catch (error) {
          if (error instanceof Error && error.message === "ESP32_NOT_REACHABLE") {
            console.log("[v0] ESP32 not reachable, simulating zone control locally")
          } else {
            console.error("[v0] Failed to control zone via API:", error)
          }

          setSystemStatus((prev) => ({
            ...prev,
            zones: prev.zones.map((z) =>
              z.zone === zone
                ? {
                    ...z,
                    state: action === "open" ? "OPENING" : "CLOSING",
                    remaining_s: action === "open" ? duration_s || 300 : 0,
                  }
                : z,
            ),
          }))

          setTimeout(() => {
            setSystemStatus((prev) => ({
              ...prev,
              zones: prev.zones.map((z) =>
                z.zone === zone
                  ? {
                      ...z,
                      state: action === "open" ? "OPEN" : "CLOSED",
                      last_watered: action === "open" ? new Date().toISOString() : z.last_watered,
                    }
                  : z,
              ),
            }))
          }, 2000)
        }
      }
    },
    [fetchStatusViaAPI],
  )

  const updateConfig = useCallback(
    async (config: Record<string, any>) => {
      const ws = wsRef.current
      console.log("[v0] Updating config:", config)

      if (ws.isConnected()) {
        ws.sendSetConfig(config)
      } else {
        try {
          await irrigationAPI.updateConfig(config)
          console.log("[v0] Config updated via API")

          setTimeout(() => fetchStatusViaAPI(), 1000)
        } catch (error) {
          if (error instanceof Error && error.message === "ESP32_NOT_REACHABLE") {
            console.log("[v0] ESP32 not reachable, config update simulated locally")
          } else {
            console.error("[v0] Failed to update config via API:", error)
          }
        }
      }
    },
    [fetchStatusViaAPI],
  )

  const requestLogs = useCallback((limit = 100) => {
    const ws = wsRef.current
    console.log("[v0] Requesting logs:", limit)

    if (ws.isConnected()) {
      ws.sendRequestLogs(limit)
    } else {
      console.warn("[v0] Cannot request logs - WebSocket not connected")
    }
  }, [])

  const reconnect = useCallback(() => {
    const ws = wsRef.current
    console.log("[v0] Manual reconnect requested")
    ws.connect().catch((error) => {
      console.error("[v0] Manual reconnect failed:", error)
    })
  }, [])

  const setRTCTime = useCallback(
    async (dateTime: {
      year: number
      month: number
      day: number
      dayOfWeek: number
      hour: number
      minute: number
      second: number
    }) => {
      try {
        const result = await irrigationAPI.setRTCDateTime(dateTime)
        console.log("[v0] RTC time set via API:", result)

        setTimeout(() => fetchStatusViaAPI(), 1000)

        return result
      } catch (error) {
        if (error instanceof Error && error.message === "ESP32_NOT_REACHABLE") {
          console.log("[v0] ESP32 not reachable, RTC time update simulated")
          return { status: "success", message: "RTC time updated (demo mode)" }
        }
        console.error("[v0] Failed to set RTC time via API:", error)
        throw error
      }
    },
    [fetchStatusViaAPI],
  )

  const emergencyStop = useCallback(async () => {
    try {
      const result = await irrigationAPI.emergencyStop()
      console.log("[v0] Emergency stop via API:", result)

      setTimeout(() => fetchStatusViaAPI(), 500)

      return result
    } catch (error) {
      if (error instanceof Error && error.message === "ESP32_NOT_REACHABLE") {
        console.log("[v0] ESP32 not reachable, emergency stop simulated")

        // Simulate emergency stop by closing all zones
        setSystemStatus((prev) => ({
          ...prev,
          zones: prev.zones.map((z) => ({ ...z, state: "CLOSED" as const, remaining_s: 0 })),
          cycle: "STOPPED",
        }))

        return { status: "success", message: "Emergency stop executed (demo mode)" }
      }
      console.error("[v0] Failed to emergency stop via API:", error)
      throw error
    }
  }, [fetchStatusViaAPI])

  return {
    isConnected,
    connectionState,
    systemStatus,
    logs,
    apiStatus,
    lastApiUpdate,
    controlZone,
    updateConfig,
    requestLogs,
    reconnect,
    setRTCTime,
    emergencyStop,
    fetchStatusViaAPI,
  }
}
