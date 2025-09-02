#!/usr/bin/env node
require('dotenv').config();
const fs = require('fs');
const path = require('path');

// Validate environment variables
const host = process.env.ESP32_HOST;
const port = process.env.ESP32_PORT || 80;
const https = process.env.ESP32_HTTPS === 'true' ? true : false;

if (!host) {
  console.error("ESP32_HOST not set. Create web/.env from web/.env.example or export env vars.");
  process.exit(1);
}

console.log(`Using ESP32 at ${https ? 'https' : 'http'}://${host}:${port}`);
const https = require('https');
const http = require('http');
const FormData = require('form-data');

// Validate environment variables
function validateEnv() {
  const requiredVars = ['ESP32_HOST'];
  const missing = requiredVars.filter(v => !process.env[v]);
  
  if (missing.length > 0) {
    console.error(`Error: Missing required environment variables: ${missing.join(', ')}`);
    console.log('Create a .env file with:');
    console.log('ESP32_HOST=your.esp32.ip.address');
    console.log('ESP32_PORT=80 # default');
    console.log('ESP32_HTTPS=false # default');
    process.exit(1);
  }
}

async function deployToESP32() {
  validateEnv();
  
  const host = process.env.ESP32_HOST;
  const port = process.env.ESP32_PORT || '80';
  const useHttps = (process.env.ESP32_HTTPS || 'false').toLowerCase() === 'true';
  const protocol = useHttps ? https : http;
  
  const buildDir = path.join(__dirname, '../../out');
  const form = new FormData();
  
  // Add all files from the build directory
  fs.readdirSync(buildDir).forEach(file => {
    const filePath = path.join(buildDir, file);
    if (fs.statSync(filePath).isFile()) {
      form.append('files', fs.createReadStream(filePath), file);
    }
  });
  
  const options = {
    host,
    port,
    path: '/update',
    method: 'POST',
    headers: form.getHeaders(),
  };
  
  console.log(`Uploading to ${host}:${port} via ${useHttps ? 'HTTPS' : 'HTTP'}`);
  
  const req = protocol.request(options, res => {
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
      console.log('Upload complete:', res.statusCode);
      console.log('Response:', data);
    });
  });
  
  req.on('error', err => {
    console.error('Upload failed:', err.message);
  });
  
  form.pipe(req);
}

deployToESP32().catch(err => console.error(err));
