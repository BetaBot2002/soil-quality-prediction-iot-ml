#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// DHT22 sensor simulation
// This chip simulates a DHT22 temperature and humidity sensor

// Global variables to store temperature and humidity values
float temperature = 30.0; // Default temperature in Celsius (range: 20-40°C)
float humidity = 60.0;    // Default humidity in percentage (range: 40-80%)

// Function to generate random fluctuations for more realistic readings
float add_fluctuation(float value, float min_fluctuation, float max_fluctuation) {
  float fluctuation = min_fluctuation + ((float)rand() / (float)RAND_MAX) * (max_fluctuation - min_fluctuation);
  return value + fluctuation;
}

// Initialize the sensor
void chip_init() {
  // Seed the random number generator
  srand(time(NULL));
  
  // Initialize with slightly randomized values
  temperature = add_fluctuation(temperature, -2.0, 2.0);
  humidity = add_fluctuation(humidity, -5.0, 5.0);
  
  printf("DHT22 sensor initialized. Initial values: Temperature=%.1f°C, Humidity=%.1f%%\n", 
         temperature, humidity);
}

// Process incoming commands
void chip_tick() {
  // Update sensor values with small random fluctuations to simulate real-world changes
  temperature = add_fluctuation(temperature, -0.2, 0.2);
  // Keep temperature in the dataset range
  if (temperature < 20.0) temperature = 20.0;
  if (temperature > 40.0) temperature = 40.0;
  
  humidity = add_fluctuation(humidity, -0.5, 0.5);
  // Keep humidity in the dataset range (40-80%)
  if (humidity < 40.0) humidity = 40.0;
  if (humidity > 80.0) humidity = 80.0;
}

// Global variables to track communication state
static uint8_t data_buffer[5]; // DHT22 sends 5 bytes of data
static int state = 0;
static int bit_counter = 0;
static int last_value = 1;
static unsigned long last_change_time = 0;

// Prepare the data buffer with current temperature and humidity values
void prepare_data_buffer() {
  // Convert temperature and humidity to the format DHT22 would send
  uint16_t humid_int = (uint16_t)(humidity * 10);
  uint16_t temp_int = (uint16_t)(temperature * 10);
  
  // DHT22 sends negative temperatures with the MSB set in the temperature word
  if (temperature < 0) {
    temp_int = (uint16_t)(-temperature * 10);
    temp_int |= 0x8000; // Set the sign bit
  }
  
  // Fill the data buffer
  data_buffer[0] = humid_int >> 8;    // Humidity high byte
  data_buffer[1] = humid_int & 0xFF;  // Humidity low byte
  data_buffer[2] = temp_int >> 8;     // Temperature high byte
  data_buffer[3] = temp_int & 0xFF;   // Temperature low byte
  data_buffer[4] = (data_buffer[0] + data_buffer[1] + data_buffer[2] + data_buffer[3]) & 0xFF; // Checksum
}

// Handle data requests
uint8_t chip_read_pin(uint8_t pin) {
  // Only respond on the DATA pin
  if (pin != 1) return 0;
  
  // Default to HIGH when idle
  if (state == 0) {
    return 1;
  }
  
  // State machine to simulate DHT22 protocol
  switch (state) {
    case 1: // Host start signal detected, respond with LOW for 80µs
      return 0;
    
    case 2: // Response signal HIGH for 80µs
      return 1;
    
    case 3: // Data transmission
      if (bit_counter < 40) { // 40 bits of data (5 bytes)
        // Each bit starts with a 50µs LOW pulse
        if (last_value == 1) {
          last_value = 0;
          return 0;
        } else {
          // Followed by a HIGH pulse - 26-28µs for '0', 70µs for '1'
          last_value = 1;
          
          // Get the current bit
          int byte_index = bit_counter / 8;
          int bit_index = 7 - (bit_counter % 8); // MSB first
          int bit_value = (data_buffer[byte_index] >> bit_index) & 0x01;
          
          bit_counter++;
          return 1; // Return HIGH - the DHT library measures the duration
        }
      } else {
        // All bits sent, go back to idle
        state = 0;
        bit_counter = 0;
        last_value = 1;
        return 1;
      }
    
    default:
      return 1; // Default to HIGH
  }
}

// Return sensor data when requested
void chip_write_pin(uint8_t pin, uint8_t value) {
  // Only respond to the DATA pin
  if (pin != 1) return;
  
  unsigned long current_time = time(NULL);
  
  // Detect start signal (pin goes from HIGH to LOW)
  if (value == 0 && last_value == 1) {
    // Host is starting communication
    state = 1;
    last_change_time = current_time;
    last_value = value;
    
    // Prepare fresh data
    prepare_data_buffer();
    
    printf("DHT22 received start signal. Current readings: Temperature=%.1f°C, Humidity=%.1f%%\n", 
           temperature, humidity);
  }
  // Detect end of start signal (pin goes from LOW to HIGH)
  else if (value == 1 && last_value == 0) {
    // Host has finished the start signal, now we respond
    if (state == 1) {
      state = 2; // Move to response state
      
      // After a short delay, move to data transmission state
      if (current_time - last_change_time >= 1) {
        state = 3;
        bit_counter = 0;
        last_value = 1; // Start with HIGH
      }
    }
    
    last_change_time = current_time;
    last_value = value;
  }
}