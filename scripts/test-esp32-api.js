const http = require("http")
const https = require("https")

// Configuration
const ESP32_HOST = process.env.ESP32_HOST || "192.168.1.100"
const ESP32_PORT = process.env.ESP32_PORT || 80
const USE_HTTPS = process.env.ESP32_HTTPS === "true"

const httpModule = USE_HTTPS ? https : http
const protocol = USE_HTTPS ? "https" : "http"

console.log(`\n🧪 Testing ESP32 API at ${protocol}://${ESP32_HOST}:${ESP32_PORT}\n`)

// Test endpoints
const endpoints = [
  { path: "/api/status", method: "GET", description: "System Status" },
  { path: "/api/config", method: "GET", description: "System Configuration" },
  { path: "/api/config/defaults", method: "GET", description: "Default Configuration" },
  { path: "/api/config/backup", method: "GET", description: "Backup List" },
]

async function testEndpoint(endpoint) {
  return new Promise((resolve) => {
    const options = {
      hostname: ESP32_HOST,
      port: ESP32_PORT,
      path: endpoint.path,
      method: endpoint.method,
      timeout: 5000,
      headers: {
        "Content-Type": "application/json",
        "User-Agent": "RiegoInteligente-WebTest/3.2",
      },
    }

    const req = httpModule.request(options, (res) => {
      let data = ""

      res.on("data", (chunk) => {
        data += chunk
      })

      res.on("end", () => {
        const result = {
          endpoint: endpoint.path,
          description: endpoint.description,
          status: res.statusCode,
          success: res.statusCode >= 200 && res.statusCode < 300,
          data: data,
          headers: res.headers,
        }
        resolve(result)
      })
    })

    req.on("error", (error) => {
      resolve({
        endpoint: endpoint.path,
        description: endpoint.description,
        status: 0,
        success: false,
        error: error.message,
      })
    })

    req.on("timeout", () => {
      req.destroy()
      resolve({
        endpoint: endpoint.path,
        description: endpoint.description,
        status: 0,
        success: false,
        error: "Request timeout",
      })
    })

    req.end()
  })
}

async function runTests() {
  console.log("📡 Starting API tests...\n")

  const results = []

  for (const endpoint of endpoints) {
    process.stdout.write(`Testing ${endpoint.description}... `)
    const result = await testEndpoint(endpoint)
    results.push(result)

    if (result.success) {
      console.log(`✅ ${result.status}`)
    } else {
      console.log(`❌ ${result.error || result.status}`)
    }
  }

  // Summary
  console.log("\n📊 Test Summary:")
  console.log("================")

  const successful = results.filter((r) => r.success).length
  const total = results.length

  console.log(`✅ Successful: ${successful}/${total}`)
  console.log(`❌ Failed: ${total - successful}/${total}`)

  if (successful === total) {
    console.log("\n🎉 All tests passed! ESP32 API is responding correctly.")
  } else {
    console.log("\n⚠️  Some tests failed. Check ESP32 connection and configuration.")
  }

  // Detailed results
  console.log("\n📋 Detailed Results:")
  console.log("====================")

  results.forEach((result) => {
    console.log(`\n${result.description} (${result.endpoint}):`)
    console.log(`  Status: ${result.status}`)
    console.log(`  Success: ${result.success ? "✅" : "❌"}`)

    if (result.error) {
      console.log(`  Error: ${result.error}`)
    }

    if (result.success && result.data) {
      try {
        const jsonData = JSON.parse(result.data)
        console.log(`  Response: ${JSON.stringify(jsonData, null, 2).substring(0, 200)}...`)
      } catch (e) {
        console.log(`  Response: ${result.data.substring(0, 100)}...`)
      }
    }
  })

  console.log("\n🔧 Development Tips:")
  console.log("===================")
  console.log("• Set ESP32_HOST environment variable to change target IP")
  console.log("• Set ESP32_PORT environment variable to change target port")
  console.log("• Set ESP32_HTTPS=true for HTTPS connections")
  console.log("• Example: ESP32_HOST=192.168.1.50 npm run test:api")
}

// Handle process termination
process.on("SIGINT", () => {
  console.log("\n\n🛑 Test interrupted by user")
  process.exit(0)
})

process.on("uncaughtException", (error) => {
  console.error("\n❌ Uncaught exception:", error.message)
  process.exit(1)
})

// Run tests
runTests().catch((error) => {
  console.error("\n❌ Test execution failed:", error.message)
  process.exit(1)
})
