# DTMF Listener - Wiring Guide

Simple wiring between ESP32+Display board and INMP441 microphone.

## Required Hardware

- ESP32 board with integrated display (Heltec/TTGO/LILYGO)
- INMP441 I2S microphone module
- 5 jumper wires

## Wiring Connections

| ESP32 Pin | INMP441 Pin | Function        | Wire Color |
| --------- | ----------- | --------------- | ---------- |
| 3.3V      | VDD         | Power           | Red        |
| GND       | GND         | Ground          | Black      |
| GND       | L/R         | Channel Select  | Black      |
| GPIO 25   | WS          | I2S Word Select | Yellow     |
| GPIO 33   | SCK         | I2S Clock       | Green      |
| GPIO 34   | SD          | I2S Data        | Blue       |

## Board-Specific Display Pins

**Heltec WiFi Kit 32:**
- SDA: GPIO 4 (pre-connected)
- SCL: GPIO 15 (pre-connected)  
- RST: GPIO 16 (pre-connected)

**TTGO T-Display:**
- CS: GPIO 5 (pre-connected)
- DC: GPIO 16 (pre-connected)
- RST: GPIO 23 (pre-connected)

**LILYGO T5 V2.3:**
- CS: GPIO 5 (pre-connected)
- DC: GPIO 17 (pre-connected)
- RST: GPIO 16 (pre-connected)

## Assembly Steps

1. **Power**: Connect ESP32 3.3V to INMP441 VDD (red wire)
2. **Ground**: Connect ESP32 GND to INMP441 GND (black wire)
3. **Channel**: Connect ESP32 GND to INMP441 L/R (black wire)
4. **I2S Signals**: 
   - GPIO 25 → WS (yellow)
   - GPIO 33 → SCK (green)  
   - GPIO 34 → SD (blue)
5. **Test**: Upload firmware and test with DTMF tones

## Troubleshooting

**No Power**: Check 3.3V on INMP441 VDD pin

**No Audio**: Verify L/R pin connected to GND

**No Display**: Check correct board selected in Arduino IDE

**Poor Detection**: Increase tone volume, check wire connections 