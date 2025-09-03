#!/usr/bin/env bash
# Initial flash via USB for esp32doit-devkit-v1
set -e
ENV=esp32doit-usb
echo "Building..."
pio run -e $ENV
echo "Uploading..."
pio run -e $ENV -t upload
echo "Done."
