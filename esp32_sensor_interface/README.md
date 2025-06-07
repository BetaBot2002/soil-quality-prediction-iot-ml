# ESP32 Soil Testing Sensor Interface

This program interfaces with an ESP32 microcontroller to collect soil data using various sensors and send it to the soil testing AI model.

## Required Sensors

1. **Temperature and Humidity Sensor (DHT22 or DHT11)**
   - Measures ambient temperature (°C)
   - Measures relative humidity (%)
   - Operating range: -40°C to 80°C
   - Humidity range: 0-100%

2. **Soil Moisture Sensor**
   - Measures soil moisture content (%)
   - Operating voltage: 3.3V-5V
   - Analog output
   - Waterproof design

3. **NPK Sensor**
   - Measures Nitrogen (N), Phosphorous (P), and Potassium (K) levels
   - Operating voltage: 9V-24V
   - RS485 interface
   - Measurement range: 0-1999mg/kg

4. **Soil Type Sensor (Optional)**
   - Can be replaced with manual input
   - Uses electrical conductivity to estimate soil type
   - Helps classify soil as Clayey, Loamy, Red, or Sandy

## Hardware Requirements

1. ESP32 Development Board
2. Sensor connections:
   - DHT22/DHT11: GPIO 4
   - Soil Moisture: GPIO 36 (ADC1_CH0)
   - NPK Sensor: GPIO 16 (RX2), GPIO 17 (TX2)
   - Soil Type Sensor: GPIO 39 (ADC1_CH3)

## Software Requirements

1. Arduino IDE or PlatformIO
2. Required Libraries:
   - DHT sensor library
   - ModbusMaster (for NPK sensor)
   - ArduinoJson

## Setup Instructions

1. Connect the sensors to ESP32 according to the pin configuration
2. Install required libraries
3. Upload the code to ESP32
4. Configure WiFi credentials in `config.h`
5. Run the Python server to receive data

## Data Flow

1. ESP32 collects sensor data every 5 minutes
2. Data is sent to local server via WiFi
3. Server processes data and feeds it to the AI model
4. Results are displayed on web interface

## Pin Configuration

```
ESP32 Pin | Sensor Connection
----------|------------------
3.3V      | VCC (all sensors)
GND       | GND (all sensors)
GPIO 4    | DHT22 Data
GPIO 36   | Soil Moisture AO
GPIO 16   | NPK Sensor RX
GPIO 17   | NPK Sensor TX
GPIO 39   | Soil Type AO
``` 