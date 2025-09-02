#!/usr/bin/env node
require('dotenv').config();
const https = require('https');
const http = require('http');

// Validate environment variables
const host = process.env.ESP32_HOST;
const port = process.env.ESP32_PORT || 80;
const httpsEnabled = process.env.ESP32_HTTPS === 'true' ? true : false;

if (!host) {
  console.error("ESP32_HOST not set. Create web/.env from web/.env.example or export env vars.");
  process.exit(1);
}

console.log(`Testing ESP32 at ${httpsEnabled ? 'https' : 'http'}://${host}:${port}`);
