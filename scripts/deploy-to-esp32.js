const fs = require("fs")
const path = require("path")
const http = require("http")
const FormData = require("form-data")

// Configuration
const ESP32_HOST = process.env.ESP32_HOST || "192.168.1.100"
const ESP32_PORT = process.env.ESP32_PORT || 80
const BUILD_DIR = path.join(__dirname, "..", "out")
const SPIFFS_UPLOAD_ENDPOINT = "/upload"

console.log(`\n🚀 Deploying to ESP32 at http://${ESP32_HOST}:${ESP32_PORT}\n`)

// Check if build directory exists
if (!fs.existsSync(BUILD_DIR)) {
  console.error('❌ Build directory not found. Run "npm run build:production" first.')
  process.exit(1)
}

// Files to deploy to ESP32 SPIFFS
const filesToDeploy = [
  { local: "index.html", remote: "/index.html" },
  { local: "_next/static/css", remote: "/css", isDirectory: true },
  { local: "_next/static/js", remote: "/js", isDirectory: true },
  { local: "favicon.ico", remote: "/favicon.ico" },
]

async function uploadFile(localPath, remotePath) {
  return new Promise((resolve, reject) => {
    const fullLocalPath = path.join(BUILD_DIR, localPath)

    if (!fs.existsSync(fullLocalPath)) {
      console.log(`⚠️  Skipping ${localPath} (not found)`)
      resolve(false)
      return
    }

    const form = new FormData()
    form.append("file", fs.createReadStream(fullLocalPath))
    form.append("path", remotePath)

    const options = {
      hostname: ESP32_HOST,
      port: ESP32_PORT,
      path: SPIFFS_UPLOAD_ENDPOINT,
      method: "POST",
      headers: form.getHeaders(),
      timeout: 30000,
    }

    const req = http.request(options, (res) => {
      let data = ""

      res.on("data", (chunk) => {
        data += chunk
      })

      res.on("end", () => {
        if (res.statusCode >= 200 && res.statusCode < 300) {
          resolve(true)
        } else {
          reject(new Error(`Upload failed with status ${res.statusCode}: ${data}`))
        }
      })
    })

    req.on("error", (error) => {
      reject(error)
    })

    req.on("timeout", () => {
      req.destroy()
      reject(new Error("Upload timeout"))
    })

    form.pipe(req)
  })
}

async function deployFiles() {
  console.log("📦 Starting deployment...\n")

  let successful = 0
  let failed = 0

  for (const file of filesToDeploy) {
    try {
      process.stdout.write(`Uploading ${file.local}... `)

      const success = await uploadFile(file.local, file.remote)

      if (success) {
        console.log("✅")
        successful++
      } else {
        console.log("⚠️  (skipped)")
      }
    } catch (error) {
      console.log(`❌ ${error.message}`)
      failed++
    }
  }

  // Summary
  console.log("\n📊 Deployment Summary:")
  console.log("======================")
  console.log(`✅ Successful: ${successful}`)
  console.log(`❌ Failed: ${failed}`)
  console.log(`⚠️  Skipped: ${filesToDeploy.length - successful - failed}`)

  if (failed === 0) {
    console.log("\n🎉 Deployment completed successfully!")
    console.log(`🌐 Access your application at: http://${ESP32_HOST}`)
  } else {
    console.log("\n⚠️  Deployment completed with errors.")
    console.log("Check ESP32 logs and SPIFFS available space.")
  }
}

// Handle process termination
process.on("SIGINT", () => {
  console.log("\n\n🛑 Deployment interrupted by user")
  process.exit(0)
})

// Check ESP32 connectivity first
console.log("🔍 Checking ESP32 connectivity...")

const healthCheck = http.request(
  {
    hostname: ESP32_HOST,
    port: ESP32_PORT,
    path: "/api/status",
    method: "GET",
    timeout: 5000,
  },
  (res) => {
    console.log(`✅ ESP32 is reachable (HTTP ${res.statusCode})\n`)
    deployFiles().catch((error) => {
      console.error("\n❌ Deployment failed:", error.message)
      process.exit(1)
    })
  },
)

healthCheck.on("error", (error) => {
  console.error(`❌ Cannot reach ESP32: ${error.message}`)
  console.error("Make sure ESP32 is connected and accessible.")
  process.exit(1)
})

healthCheck.on("timeout", () => {
  console.error("❌ ESP32 connection timeout")
  console.error("Check network connectivity and ESP32 status.")
  process.exit(1)
})

healthCheck.end()
