# Arduino IDE Setup Guide

This project can be used with both PlatformIO and Arduino IDE. Follow these steps for Arduino IDE setup:

## Quick Setup for Arduino IDE

1. **Install ESP32 Board Package**
   - Open Arduino IDE
   - Go to `File → Preferences`
   - Add to "Additional Board Manager URLs": 
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Go to `Tools → Board → Boards Manager`
   - Search for "ESP32" and install "ESP32 by Espressif Systems"

2. **Install Required Libraries**
   - Go to `Sketch → Include Library → Manage Libraries`
   - Search and install:
     - `Adafruit SSD1306`
     - `Adafruit GFX Library`

3. **Open the Project**
   - Copy the contents of `src/main.cpp` to a new Arduino sketch
   - Save the sketch as `dtmf_listener.ino`

4. **Board Configuration**
   - Select `Tools → Board → ESP32 Arduino → ESP32 Dev Module`
   - Select the correct port under `Tools → Port`
   - Set upload speed to `115200` under `Tools → Upload Speed`

5. **Upload and Run**
   - Click the upload button
   - Open Serial Monitor at 115200 baud rate

## Alternative: Use PlatformIO

If you prefer PlatformIO (recommended for advanced users):

1. Install [PlatformIO](https://platformio.org/platformio-ide)
2. Open this project folder in PlatformIO
3. Build and upload using PlatformIO commands

The `platformio.ini` file contains all necessary configurations. 