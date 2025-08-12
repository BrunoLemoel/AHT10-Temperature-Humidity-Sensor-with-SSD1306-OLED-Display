# AHT10-Temperature-Humidity-Sensor-with-SSD1306-OLED-Display
A comprehensive IoT project for Raspberry Pi Pico W featuring real-time environmental monitoring with visual alerts and OLED display integration.

![20250731_214038](https://github.com/user-attachments/assets/71f7e02b-71ed-4c0a-95e5-abefe4d2e903)


🌡️ Project Overview
This project implements a professional-grade temperature and humidity monitoring system using the AHT10 sensor and SSD1306 OLED display. The system provides real-time environmental data with intelligent alerting, optimized for indoor climate monitoring applications.

✨ Key Features
🔧 Hardware Integration
Raspberry Pi Pico W: Main microcontroller with dual I2C bus configuration
AHT10 Sensor: High-precision temperature (-40°C to +85°C) and humidity (0-100% RH) measurements
SSD1306 OLED Display: 128x64 monochrome display with custom bitmap fonts
Dual I2C Configuration: Optimized bus separation for sensor and display communication

📊 Smart Monitoring
Real-time Data: Continuous temperature and humidity monitoring every 2 seconds
Environmental Compensation: Automatic temperature calibration (-2.3°C offset) for improved accuracy
Comfort Level Analysis: Intelligent classification (IDEAL, HOT, COLD, DRY, HUMID)

🚨 Visual Alert System
Conditional Alerts: Dynamic warning symbols for critical conditions
!C - Low temperature alert (< 20°C)
!Q - High temperature alert (> 40°C)
!H - High humidity alert (> 70%)
Smart Display: Alerts appear only when thresholds are exceeded
Inverted Text: Error messages with highlighted background for visibilit

💻 Advanced Display Features
Custom Bitmap Fonts: 5x8 pixel font supporting 0-9, A-Z, and special characters
Optimized Layout: 4-line display layout perfectly fitted for 128x64 resolution
Error Handling: Comprehensive error screens with diagnostic messages
Startup Screen: Professional initialization sequence display

🔌 Hardware Configuration
Pin Connections
AHT10 Sensor (I2C0):
├── VCC → 3.3V
├── GND → Ground
├── SDA → GPIO 0
└── SCL → GPIO 1

SSD1306 Display (I2C1):
├── VCC → 3.3V
├── GND → Ground
├── SDA → GPIO 14
└── SCL → GPIO 15

I2C Addresses
AHT10: 0x38 (I2C0 Bus)
SSD1306: 0x3C (I2C1 Bus)

🛠️ Technical Specifications
Software Architecture
Language: C/C++ with Pico SDK 1.5.1
Build System: CMake with Ninja generator
Real-time OS: FreeRTOS integration ready

Memory Management: Optimized buffer handling for display operations
Performance Metrics
Update Rate: 2-second sensor reading cycle

Display Refresh: Smart 5-cycle update interval to prevent flickering
Accuracy: ±0.3°C temperature, ±2% humidity (after compensation)

Response Time: <8 seconds for environmental changes

Quick Setup
Hardware Assembly: Connect AHT10 and SSD1306 according to pin diagram
Power On: Connect Pico W via USB
Flash Firmware: Use generated .uf2 file
Monitor Output: View serial console for debugging info

📱 User Interface
┌─────────────────────────┐
│ AHT10                   │ ← System identifier
│ 26.0C                   │ ← Temperature (with alerts)
│ 65.3%                   │ ← Humidity (with alerts)  
│ IDEAL                   │ ← Environmental status
└─────────────────────────┘

Alert Examples
Normal Operation:        Critical Conditions:
┌─────────────────────────┐ ┌─────────────────────────┐
│ AHT10                   │ │ AHT10                   │
│ 24.2C                   │ │ 18.5C  !C               │
│ 55.8%                   │ │ 75.2%  !H               │
│ IDEAL                   │ │ FRIO                    │
└─────────────────────────┘ └─────────────────────────┘

🔬 Advanced Features
Environmental Compensation
The system implements sophisticated temperature compensation to account for:

Microcontroller self-heating effects
Local microclimate variations
Sensor placement considerations
Error Recovery
Automatic Sensor Detection: Continuous retry mechanism for sensor connectivity
Graceful Degradation: System continues operation even with display failures
Diagnostic Messages: Comprehensive error reporting via serial and display
Memory Optimization
Efficient Bitmap Rendering: Custom font system optimized for SSD1306
Smart Buffer Management: Minimal RAM usage with strategic buffer allocation
Display Caching: Reduced I2C traffic through intelligent update cycles

📈 Applications
Ideal Use Cases
Smart Home Monitoring: Room climate control and optimization
Server Room Management: Equipment temperature monitoring
Greenhouse Automation: Plant growth environment tracking
Laboratory Conditions: Precision environmental control
Weather Stations: Local microclimate data collection
Integration Potential
IoT Connectivity: WiFi data logging and remote monitoring
Home Assistant: MQTT integration for smart home systems
Data Logging: SD card storage for historical analysis
Alert Systems: Email/SMS notifications for critical conditions

🤝 Contributing
We welcome contributions! Please see our contributing guidelines for:

Code style standards
Testing procedures
Documentation requirements
Pull request workflow

