import serial
import time

# Configure the serial port - disable DTR and RTS to prevent ESP32 reset
ser = serial.Serial(
    port='COM3',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1,
    dsrdtr=False,  # Disable DTR to prevent reset
    rtscts=False   # Disable RTS to prevent reset
)
ser.close()  # Ensure port is closed initially

# RTC configuration data (September 1, 2025, 8:58:35, Monday)
# Format: AAMMDDWHHMMSS (AA=year, MM=month, DD=day, W=weekday, HH=hour, MM=minute, SS=second)
rtc_config = "2509011085835"

try:
    ser.open()
    time.sleep(2)  # Wait for connection to establish
    # Send the 13 digits followed by a newline character
    ser.write(rtc_config.encode() + b'\n')
    print(f"Sent RTC configuration: {rtc_config}")
    time.sleep(1)
    response = ser.read_all()
    print(f"Response (raw bytes): {response}")
    try:
        print(f"Response (decoded): {response.decode()}")
    except UnicodeDecodeError:
        print("Response contains binary data that cannot be decoded as UTF-8")
finally:
    ser.close()
