"use client"

export interface IrrigationStatus {
  state: string
  activeZone: number
  remainingTime: number
  memoryFree: number
  status: string
}

export interface RTCConfig {
  year: number
  month: number
  day: number
  dayOfWeek: number
  hour: number
  minute: number
  second: number
}

export interface ZoneConfig {
  id: number
  name: string
  gpio: number
  wateringTime: number
  enabled: boolean
  servoOpenAngle: number
  servoCloseAngle: number
}

export interface SystemConfig {
  irrigation: {
    zones: Array<{
      id: number
      name: string
      duration: number
      enabled: boolean
    }>
    schedule: {
      enabled: boolean
      startTime: string
      interval: number
    }
  }
  network: {
    ssid: string
    hostname: string
  }
  security: {
    webAuth: boolean
    username: string
  }
}

class IrrigationAPI {
  private baseUrl: string
  private credentials: { username: string; password: string } | null = null
  private apiVersion = "v1"

  constructor(baseUrl = "") {
    if (baseUrl) {
      this.baseUrl = baseUrl
    } else if (typeof window !== "undefined") {
      // Client-side: use ESP32 environment variables or fallback to current host
      const host = process.env.NEXT_PUBLIC_ESP32_HOST || window.location.hostname
      const port = process.env.NEXT_PUBLIC_ESP32_PORT || "80"
      const isHttps = process.env.NEXT_PUBLIC_ESP32_HTTPS === "true"
      const protocol = isHttps ? "https" : "http"

      // Only add port if it's not the default port for the protocol
      const portSuffix =
        (protocol === "http" && port === "80") || (protocol === "https" && port === "443") ? "" : `:${port}`

      this.baseUrl = `${protocol}://${host}${portSuffix}`
    } else {
      // Server-side: use environment variables or localhost fallback
      const host = process.env.ESP32_HOST || "localhost"
      const port = process.env.ESP32_PORT || "80"
      const isHttps = process.env.ESP32_HTTPS === "true"
      const protocol = isHttps ? "https" : "http"

      this.baseUrl = `${protocol}://${host}:${port}`
    }

    console.log(`[API] Client initialized with base URL: ${this.baseUrl}, version: ${this.apiVersion}`)
  }

  setCredentials(username: string, password: string) {
    this.credentials = { username, password }
  }

  private async makeRequest(endpoint: string, options: RequestInit = {}): Promise<Response> {
    // Ensure endpoint starts with slash and includes version
    const normalizedEndpoint = endpoint.startsWith('/') ? 
      `/api/${this.apiVersion}${endpoint}` : 
      `/api/${this.apiVersion}/${endpoint}`
      
    const url = `${this.baseUrl}${normalizedEndpoint}`

    // Add basic auth if credentials are set
    if (this.credentials) {
      const auth = btoa(`${this.credentials.username}:${this.credentials.password}`)
      options.headers = {
        ...options.headers,
        Authorization: `Basic ${auth}`,
      }
    }

    // Add default headers
    options.headers = {
      "Content-Type": "application/json",
      ...options.headers,
    }

    try {
      console.log("[v0] Making API request to:", url)
      const response = await fetch(url, options)

      if (response.status === 401) {
        throw new Error("Authentication required")
      }

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`)
      }

      return response
    } catch (error) {
      if (error instanceof TypeError && error.message.includes("fetch")) {
        console.warn(`[API] ESP32 not reachable at ${url}. Running in demo mode.`)
        throw new Error("ESP32_NOT_REACHABLE")
      }
      console.error(`[API] Error making request to ${endpoint}:`, error)
      throw error
    }
  }

  // System status endpoints
  async getStatus(): Promise<IrrigationStatus> {
    const response = await this.makeRequest("/api/status")
    return response.json()
  }

  // RTC configuration endpoints
  async setRTCDateTime(config: RTCConfig): Promise<{ status: string; message: string }> {
    const formData = new URLSearchParams()
    Object.entries(config).forEach(([key, value]) => {
      formData.append(key, value.toString())
    })

    const response = await this.makeRequest("/api/config/rtc", {
      method: "POST",
      headers: {
        "Content-Type": "application/x-www-form-urlencoded",
      },
      body: formData,
    })
    return response.json()
  }

  async restartRTC(): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/rtc/restart", {
      method: "POST",
    })
    return response.json()
  }

  // Configuration management endpoints
  async getConfig(): Promise<SystemConfig> {
    const response = await this.makeRequest("/api/config")
    return response.json()
  }

  async updateConfig(config: Partial<SystemConfig>): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/config", {
      method: "PUT",
      body: JSON.stringify(config),
    })
    return response.json()
  }

  async updateZones(zones: ZoneConfig[]): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/config/zones", {
      method: "PUT",
      body: JSON.stringify({ zones }),
    })
    return response.json()
  }

  async getZoneConfig(): Promise<{ zones: ZoneConfig[] }> {
    const response = await this.makeRequest("/api/config/zones")
    return response.json()
  }

  async updateSystemConfig(config: any): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/config/system", {
      method: "PUT",
      body: JSON.stringify(config),
    })
    return response.json()
  }

  async getDefaultConfig(): Promise<SystemConfig> {
    const response = await this.makeRequest("/api/config/defaults")
    return response.json()
  }

  async resetConfig(): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/config/reset", {
      method: "POST",
    })
    return response.json()
  }

  // Backup management endpoints
  async listBackups(): Promise<{ backups: number; last_backup: string }> {
    const response = await this.makeRequest("/api/config/backup")
    return response.json()
  }

  async createBackup(): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/config/backup", {
      method: "POST",
    })
    return response.json()
  }

  async restoreBackup(): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/config/backup/restore", {
      method: "POST",
    })
    return response.json()
  }

  // Zone control methods (for future WebSocket integration)
  async startZone(zoneId: number, duration?: number): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/zones/start", {
      method: "POST",
      body: JSON.stringify({ zoneId, duration }),
    })
    return response.json()
  }

  async stopZone(zoneId: number): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/zones/stop", {
      method: "POST",
      body: JSON.stringify({ zoneId }),
    })
    return response.json()
  }

  async emergencyStop(): Promise<{ status: string; message: string }> {
    const response = await this.makeRequest("/api/emergency/stop", {
      method: "POST",
    })
    return response.json()
  }
}

export const apiClient = new IrrigationAPI()
export const irrigationAPI = apiClient

// Export class for custom instances
export { IrrigationAPI }
