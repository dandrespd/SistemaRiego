"use client"

import { SystemConfig } from './api'

export interface WebSocketMessage {
  type: string
  [key: string]: any
}

export interface StateUpdateMessage extends WebSocketMessage {
  type: "state_update"
  zones: Array<{
    zone: number
    name: string
    state: "OPEN" | "CLOSED" | "OPENING" | "CLOSING"
    remaining_s: number
  }>
  cycle: "AUTO" | "MANUAL" | "STOPPED"
  mem_free: number
}

export interface LogEntryMessage extends WebSocketMessage {
  type: "log_entry"
  level: "INFO" | "WARN" | "ERROR"
  msg: string
  ts: string
}

export interface RTCTimeMessage extends WebSocketMessage {
  type: "rtc_time"
  iso: string
}

export interface WiFiStatusMessage extends WebSocketMessage {
  type: "wifi_status"
  ssid: string
  rssi: number
  connected: boolean
}

export interface ControlZoneMessage extends WebSocketMessage {
  type: "control_zone"
  zone: number
  action: "open" | "close"
  duration_s?: number
}

export interface SetConfigMessage extends WebSocketMessage {
  type: "set_config"
  config: Partial<SystemConfig>
}

export interface RequestLogsMessage extends WebSocketMessage {
  type: "request_logs"
  limit: number
}

export class IrrigationWebSocket {
  private ws: WebSocket | null = null
  private reconnectAttempts = 0
  private maxReconnectAttempts = 5
  private baseReconnectDelay = 1000
  private maxReconnectDelay = 30000
  private isConnecting = false
  private messageHandlers: Map<string, (message: WebSocketMessage) => void> = new Map()
  private connectionStateHandlers: Array<(connected: boolean) => void> = []
  private apiVersion = "v1"

  constructor(private baseUrl: string) {}

  connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      if (this.isConnecting || (this.ws && this.ws.readyState === WebSocket.CONNECTING)) {
        return
      }

      this.isConnecting = true

      try {
        // Connect to the firmware's WebSocket endpoint at /ws
        const base = this.baseUrl || window.location.origin
        const parsed = new URL(base)
        const wsProtocol = parsed.protocol === "https:" ? "wss" : "ws"
        // Connect to /ws (matches firmware implementation)
        const wsUrl = `${wsProtocol}://${parsed.host}/ws`
        console.log("[API] Connecting to WebSocket:", wsUrl)
        this.ws = new WebSocket(wsUrl)
        // Note: For versioning, could use /api/v1/ws in the future

        this.ws.onopen = () => {
          console.log("[API] WebSocket connected successfully")
          this.isConnecting = false
          this.reconnectAttempts = 0
          this.baseReconnectDelay = 1000
          this.notifyConnectionState(true)
          resolve()
        }

        this.ws.onmessage = (event) => {
          try {
            const message: WebSocketMessage = JSON.parse(event.data)
            console.log("[API] Received WebSocket message:", message.type, message)
            this.handleMessage(message)
          } catch (error) {
            console.error("[API] Error parsing WebSocket message:", error)
          }
        }

        this.ws.onclose = (event) => {
          console.log("[API] WebSocket connection closed:", event.code, event.reason)
          this.isConnecting = false
          this.notifyConnectionState(false)

          if (!event.wasClean && this.reconnectAttempts < this.maxReconnectAttempts) {
            this.scheduleReconnect()
          }
        }

        this.ws.onerror = (error) => {
          console.error("[API] WebSocket error:", error)
          this.isConnecting = false
          this.notifyConnectionState(false)
          reject(error)
        }
      } catch (error) {
        this.isConnecting = false
        console.error("[API] Error creating WebSocket:", error)
        reject(error)
      }
    })
  }

  private scheduleReconnect() {
    this.reconnectAttempts++
    const delay = Math.min(
      this.baseReconnectDelay * Math.pow(2, this.reconnectAttempts - 1),
      this.maxReconnectDelay
    )

      console.log(
        `[API] Scheduling reconnect attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts} in ${delay}ms`,
      )

    setTimeout(() => {
      if (this.reconnectAttempts <= this.maxReconnectAttempts) {
        this.connect().catch(() => {
          // Reconnection failed, will try again if attempts remain
        })
      }
    }, delay)
  }

  private handleMessage(message: WebSocketMessage) {
    const handler = this.messageHandlers.get(message.type)
    if (handler) {
      handler(message)
    } else {
      console.warn("[API] No handler registered for message type:", message.type)
    }
  }

  private notifyConnectionState(connected: boolean) {
    this.connectionStateHandlers.forEach((handler) => handler(connected))
  }

  onMessage<T extends WebSocketMessage>(type: string, handler: (message: T) => void) {
    this.messageHandlers.set(type, handler as (message: WebSocketMessage) => void)
  }

  onConnectionStateChange(handler: (connected: boolean) => void) {
    this.connectionStateHandlers.push(handler)
  }

  send(message: WebSocketMessage) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      console.log("[API] Sending WebSocket message:", message.type, message)
      this.ws.send(JSON.stringify(message))
    } else {
      console.warn("[API] Cannot send message - WebSocket not connected:", message)
    }
  }

  sendControlZone(zone: number, action: "open" | "close", duration_s?: number) {
    const message: ControlZoneMessage = {
      type: "control_zone",
      zone,
      action,
      ...(duration_s && { duration_s }),
    }
    this.send(message)
  }

  sendSetConfig(config: Partial<SystemConfig>) {
    const message: SetConfigMessage = {
      type: "set_config",
      config,
    }
    this.send(message)
  }

  sendRequestLogs(limit = 100) {
    const message: RequestLogsMessage = {
      type: "request_logs",
      limit,
    }
    this.send(message)
  }

  disconnect() {
    if (this.ws) {
      console.log("[API] Disconnecting WebSocket")
      this.ws.close(1000, "Client disconnect")
      this.ws = null
      this.messageHandlers.clear()
      this.connectionStateHandlers = []
    }
  }

  isConnected(): boolean {
    return this.ws !== null && this.ws.readyState === WebSocket.OPEN
  }

  getConnectionState(): "connecting" | "open" | "closing" | "closed" {
    if (!this.ws) return "closed"

    switch (this.ws.readyState) {
      case WebSocket.CONNECTING:
        return "connecting"
      case WebSocket.OPEN:
        return "open"
      case WebSocket.CLOSING:
        return "closing"
      case WebSocket.CLOSED:
        return "closed"
      default:
        return "closed"
    }
  }
}

// Create a singleton instance for the application
let wsInstance: IrrigationWebSocket | null = null

export function getWebSocketInstance(): IrrigationWebSocket {
  if (!wsInstance) {
    // Use ESP32 environment variables with proper fallbacks
    const host = process.env.NEXT_PUBLIC_ESP32_HOST || process.env.ESP32_HOST || "192.168.1.100"
    const port = process.env.NEXT_PUBLIC_ESP32_PORT || process.env.ESP32_PORT || "80"
    const isHttps = process.env.NEXT_PUBLIC_ESP32_HTTPS === "true" || process.env.ESP32_HTTPS === "true"

    // Construct base URL based on ESP32 configuration
    const protocol = isHttps ? "https" : "http"
    const portSuffix = 
      (protocol === "http" && port === "80") || (protocol === "https" && port === "443") ? "" : `:${port}`
    const baseUrl = `${protocol}://${host}${portSuffix}`

    console.log("[API] Creating WebSocket instance with base URL:", baseUrl)
    wsInstance = new IrrigationWebSocket(baseUrl)
  }
  return wsInstance
}
