# DTMF Listener

A real-time DTMF (Dual-Tone Multi-Frequency) detection system built with ESP32 that can detect and decode telephone keypad tones. The system uses digital signal processing to identify DTMF signals and displays the detected digits on an OLED screen.

## Features

- **Real-time DTMF Detection**: Detects standard telephone keypad tones (0-9, *, #, A-D)
- **Visual Feedback**: OLED display shows detected digits and signal strength
- **Audio Level Monitoring**: Real-time audio input monitoring with visual indicators
- **History Tracking**: Maintains a history of the last 12 detected digits
- **Debug Mode**: Comprehensive debugging tools for troubleshooting
- **Serial Commands**: Interactive commands for testing and configuration

## Hardware Requirements

### ESP32 Development Board
- Any ESP32 board (ESP32-WROOM, ESP32-DevKit, etc.)

### I2S Digital Microphone
- INMP441, MAX9814, or similar I2S digital microphone
- **Important**: Must be an I2S digital microphone, not analog

### OLED Display
- 128x64 SSD1306 OLED display with I2C interface

## Pin Configuration

| Component        | ESP32 Pin | Description                       |
| ---------------- | --------- | --------------------------------- |
| **Microphone**   |           |                                   |
| VDD              | 3.3V      | Power supply (**NOT 5V!**)        |
| GND              | GND       | Ground                            |
| WS               | GPIO 25   | Word Select                       |
| SCK              | GPIO 33   | Serial Clock                      |
| SD               | GPIO 34   | Serial Data (input-only pin)      |
| L/R              | GND       | Left/Right selection (GND = left) |
| **OLED Display** |           |                                   |
| VCC              | 3.3V      | Power supply                      |
| GND              | GND       | Ground                            |
| SDA              | GPIO 4    | I2C Data                          |
| SCL              | GPIO 15   | I2C Clock                         |

## Installation

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support
- Required libraries (install via Arduino Library Manager):
  - `Adafruit SSD1306`
  - `Adafruit GFX Library`

### Setup Steps

1. **Install ESP32 Board Support**
   ```
   File → Preferences → Additional Board Manager URLs
   Add: https://dl.espressif.com/dl/package_esp32_index.json
   Tools → Board → Boards Manager → Search "ESP32" → Install
   ```

2. **Install Required Libraries**
   ```
   Sketch → Include Library → Manage Libraries
   Search and install:
   - Adafruit SSD1306
   - Adafruit GFX Library
   ```

3. **Clone/Download Project**
   ```bash
   git clone https://github.com/yourusername/dtmf-listener.git
   ```

4. **Upload Code**
   - Open `src/main.cpp` in Arduino IDE
   - Select your ESP32 board: `Tools → Board → ESP32 Arduino → ESP32 Dev Module`
   - Select correct port: `Tools → Port → [Your ESP32 Port]`
   - Upload the code

## Usage

### Basic Operation

1. **Power on** the ESP32 with all connections made
2. **Monitor Serial Output** at 115200 baud for system status
3. **Generate DTMF Tones** using:
   - Phone dialer apps
   - Online DTMF tone generators
   - Actual telephone keypad
4. **View Results** on the OLED display and serial monitor

### Serial Commands

| Command | Description                   |
| ------- | ----------------------------- |
| `d`     | Toggle debug mode on/off      |
| `s`     | Show raw audio samples        |
| `t`     | Run 10-second microphone test |
| `r`     | Reset I2S audio driver        |

### Display Information

The OLED shows:
- **Large Digit**: Currently detected DTMF digit
- **Signal Levels**: Row (R) and Column (C) frequency magnitudes
- **Threshold**: Current detection threshold (T)
- **Status**: Detection status (Listening/Signal/DETECTED!)
- **History**: Last 12 detected digits

## DTMF Frequency Reference

| Key | Row Freq | Col Freq | Key | Row Freq | Col Freq |
| --- | -------- | -------- | --- | -------- | -------- |
| 1   | 697 Hz   | 1209 Hz  | 2   | 697 Hz   | 1336 Hz  |
| 3   | 697 Hz   | 1477 Hz  | A   | 697 Hz   | 1633 Hz  |
| 4   | 770 Hz   | 1209 Hz  | 5   | 770 Hz   | 1336 Hz  |
| 6   | 770 Hz   | 1477 Hz  | B   | 770 Hz   | 1633 Hz  |
| 7   | 852 Hz   | 1209 Hz  | 8   | 852 Hz   | 1336 Hz  |
| 9   | 852 Hz   | 1477 Hz  | C   | 852 Hz   | 1633 Hz  |
| *   | 941 Hz   | 1209 Hz  | 0   | 941 Hz   | 1336 Hz  |
| #   | 941 Hz   | 1477 Hz  | D   | 941 Hz   | 1633 Hz  |

## Troubleshooting

### No Audio Detection
- Verify microphone power (3.3V, **not 5V**)
- Check L/R pin connected to GND
- Ensure all I2S pin connections are secure
- Confirm microphone is I2S digital type
- Use `t` command to test microphone

### Poor Detection Accuracy
- Increase audio input volume
- Move microphone closer to audio source
- Reduce background noise
- Check for loose connections
- Try different DTMF tone generator

### Display Issues
- Verify I2C connections (SDA/SCL)
- Check OLED power supply (3.3V)
- Ensure correct I2C address (0x3C)

## Technical Details

### Signal Processing
- **Algorithm**: Goertzel algorithm for frequency detection
- **Sample Rate**: 8000 Hz
- **Buffer Size**: 1024 samples
- **Detection Method**: Dual-frequency magnitude comparison

### Configuration
- **Detection Threshold**: 1000.0 (adjustable in code)
- **Display Update Rate**: 100ms
- **History Size**: 12 digits maximum

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- DTMF detection based on Goertzel algorithm implementation
- Adafruit libraries for OLED display support
- ESP32 community for I2S audio examples

## Version History

- **v1.0**: Initial release with basic DTMF detection
  - Real-time frequency analysis
  - OLED display integration
  - Serial command interface
  - Microphone testing tools
