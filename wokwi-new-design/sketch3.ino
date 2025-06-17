#define BLYNK_TEMPLATE_ID "TMPL3grG9nPT7"
#define BLYNK_TEMPLATE_NAME "soil moisture and npk"
#define BLYNK_AUTH_TOKEN "gFxZ4mVCpGZFPmfDFYFFeOtDOb9_5hxY"

// Define WOKWI macro for simulator detection
// #define WOKWI 1  // Uncomment this line when using Wokwi simulator

// Pin definitions
#define LED_PIN 26
#define BUZZER_PIN 27
#define DHTPIN 15     // DHT22 sensor pin
#define MOISTURE_PIN 34  // Analog pin for moisture sensor

// Include necessary libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"  // DHT sensor library
#include <ThingSpeak.h>  // ThingSpeak library

// WiFi credentials
char ssid[] = "Wokwi-GUEST";  // Replace with your WiFi network name
char pass[] = "";  // Replace with your WiFi password

// ThingSpeak settings - IMPORTANT: Replace with your own values for real hardware
// Create a ThingSpeak channel at https://thingspeak.com and get your channel ID and Write API Key
unsigned long thingSpeakChannelNumber = 2989083;  // Replace with your channel number
const char* thingSpeakWriteAPIKey = "O5SNHNR7NKHX0OZZ";  // Replace with your ThingSpeak Write API Key
WiFiClient thingSpeakClient;  // Client for ThingSpeak connection

// Sensor variables
int moistureValue = 0;  // For storing moisture sensor reading

// NPK sensor communication
#define NPK_COMMANDS 3  // Number of NPK sensor commands
char npkCommands[NPK_COMMANDS] = {0x1, 0x3, 0x5};  // Commands for K, N, P
char npkLabels[NPK_COMMANDS][30] = {"Potassium value is: ", "Nitrogen value is: ", "Phosphorous value is: "};
uint8_t npkValues[NPK_COMMANDS] = {0, 0, 0};  // Store NPK sensor values

// Initialize LCD display
LiquidCrystal_I2C lcd(0x27, 20, 4);  // 20x4 LCD display at address 0x27

// Initialize DHT sensor
DHTesp dhtSensor;

// Environmental variables
float temperature = 0;
float humidity = 0;

// Variables for randomized sensor data
unsigned long lastRandomizeTime = 0;
const int randomizeInterval = 1000;  // 1 second interval for randomizing data

// Previous sensor values (for ensuring small changes)
int prevMoistureValue = 500;  // Starting with a mid-range value
float prevTemperature = 25.0;  // Starting with room temperature
float prevHumidity = 50.0;    // Starting with moderate humidity
uint8_t prevNpkValues[NPK_COMMANDS] = {50, 50, 50};  // Starting with mid-range NPK values

/**
 * Produces alert sounds with the buzzer
 * @param frequency Tone frequency in Hz
 * @param duration Duration of each beep in ms
 * @param repeat Number of beeps to produce
 */
void beepBuzzer(int frequency, int duration, int repeat) {
  for (int i = 0; i < repeat; i++) {
    tone(BUZZER_PIN, frequency, duration);
    delay(300);  // Delay between consecutive beeps
  }
  delay(2000);  // Delay after the entire sequence
}


/**
 * Setup function - runs once at startup
 */
void setup() {
  // Initialize serial communication
  Serial.begin(115200);  // Debug serial port
  Serial2.begin(15200, SERIAL_8N1, 16, 17);  // NPK sensor communication

  // Initialize random number generator
  randomSeed(analogRead(A0));
  
  // Set initial sensor values based on realistic data from dataset
  prevMoistureValue = random(20, 70);  // Moisture range 20-70%
  prevTemperature = random(200, 400) / 10.0;  // Temperature range 20-40°C
  prevHumidity = random(400, 800) / 10.0;     // Humidity range 40-80%
  // NPK values (K, N, P) based on dataset
  prevNpkValues[0] = random(15, 45);  // Potassium range 0-45
  prevNpkValues[1] = random(15, 45);  // Nitrogen range 0-45
  prevNpkValues[2] = random(15, 45);  // Phosphorous range 0-45
  
  // Initialize sensor values with the initial random values
  moistureValue = prevMoistureValue;
  temperature = prevTemperature;
  humidity = prevHumidity;
  for (uint8_t i = 0; i < NPK_COMMANDS; i++) {
    npkValues[i] = prevNpkValues[i];
  }
  
  // Connect to WiFi and Blynk
  Serial.print("Connecting to WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Wait for WiFi connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Connected to Blynk");
  } else {
    Serial.println("\nFailed to connect to WiFi. Check credentials or network.");
  }
  
  // Initialize ThingSpeak
  ThingSpeak.begin(thingSpeakClient);
  Serial.println("ThingSpeak initialized");

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil Testing System");
  
  // Initialize DHT sensor
  dhtSensor.setup(DHTPIN, DHTesp::DHT22);
  Serial.println("DHT22 sensor initialized");
  
  // Allow sensors to stabilize
  delay(2000);
}

/**
 * Sends sensor data to ThingSpeak
 * Creates a multi-field update to send all sensor readings to ThingSpeak
 */
void sendDataToThingSpeak() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected for ThingSpeak");
    return;
  }
  
  // Set the fields with our sensor values
  ThingSpeak.setField(1, humidity);       // Field 1: Moisture
  ThingSpeak.setField(2, temperature);         // Field 2: Temperature
  ThingSpeak.setField(3, moistureValue);            // Field 3: Humidity
  ThingSpeak.setField(4, npkValues[1]);        // Field 4: Nitrogen
  ThingSpeak.setField(5, npkValues[2]);        // Field 5: Phosphorus
  ThingSpeak.setField(6, npkValues[0]);        // Field 6: Potassium
  
  // Log connection attempt
  Serial.println("Sending data to ThingSpeak...");
  
  // Write to all fields at once
  int httpCode = ThingSpeak.writeFields(thingSpeakChannelNumber, thingSpeakWriteAPIKey);
  
  // Check the return code
  if (httpCode == 200) {
    Serial.println("ThingSpeak update successful");
    
    // Update LCD with success notice
    lcd.setCursor(0, 3);
    lcd.print("TS: Data Sent OK");
  } else {
    Serial.println("ThingSpeak error: " + String(httpCode));
    
    // Update LCD with error notice
    lcd.setCursor(0, 3);
    lcd.print("TS Error: " + String(httpCode));
  }
}

/**
 * Main loop function - runs repeatedly
 */
void loop() {
  Blynk.run();  // Process Blynk communications
  
  // Check if it's time to randomize sensor data (every 1 second)
  unsigned long currentMillis = millis();
  if (currentMillis - lastRandomizeTime >= randomizeInterval) {
    lastRandomizeTime = currentMillis;
    randomizeSensorData();
  }
  
  // The rest of the loop continues with the existing 15-second interval
  // for ThingSpeak updates and other operations
  
  // Check alert conditions
  bool moistureAlert = (moistureValue < 20);
  bool nutrientAlert = (npkValues[0] < 10 || npkValues[1] < 10 || npkValues[2] < 10);
  bool climateAlert = (temperature > 35.0 || humidity < 30.0);
  bool anyAlert = moistureAlert || nutrientAlert || climateAlert;
  
  // Trigger alerts if needed
  if (anyAlert) {
    digitalWrite(LED_PIN, HIGH);  // Turn on warning LED
    beepBuzzer(3000, 1000, 3);    // Sound alarm
    Blynk.virtualWrite(V4, HIGH); // Alert indicator in Blynk
  } else {
    digitalWrite(LED_PIN, LOW);   // Turn off warning LED
    Blynk.virtualWrite(V4, LOW);  // Clear alert in Blynk
  }

  // Update LCD display
  lcd.clear();
  
  // Row 0: Moisture and Temperature
  lcd.setCursor(0, 0);
  lcd.print("Moist:");
  lcd.print(moistureValue);
  lcd.print(" T:");
  lcd.print(temperature, 1);
  lcd.print("C");
  
  // Row 1: Humidity
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 1);
  lcd.print("%");
  
  // Row 2: NPK values
  lcd.setCursor(0, 2);
  lcd.print("N:");
  lcd.print(npkValues[1]);
  lcd.print(" P:");
  lcd.print(npkValues[2]);
  lcd.print(" K:");
  lcd.print(npkValues[0]);
  
  // Row 3: Status message
  lcd.setCursor(0, 3);
  if (climateAlert) {
    lcd.print("Climate Alert!");
  } else if (moistureAlert) {
    lcd.print("Moisture Alert!");
  } else if (nutrientAlert) {
    lcd.print("Nutrient Alert!");
  } else {
    lcd.print("All Parameters OK");
  }

  // Send data to Blynk
  Blynk.virtualWrite(V0, moistureValue);  // Moisture
  Blynk.virtualWrite(V1, npkValues[1]);   // Nitrogen
  Blynk.virtualWrite(V2, npkValues[2]);   // Phosphorus
  Blynk.virtualWrite(V3, npkValues[0]);   // Potassium
  Blynk.virtualWrite(V5, temperature);    // Temperature
  Blynk.virtualWrite(V6, humidity);       // Humidity
  
  // Send data to ThingSpeak
  sendDataToThingSpeak();
  
  // Wait before next reading
  delay(15000);  // ThingSpeak free tier has a 15-second update limit
}


/**
 * Randomizes sensor data with small variations from previous values
 * Ensures new values don't differ too much from previous readings
 */
void randomizeSensorData() {
  // Random variation limits
  const int moistureVariation = 5;     // Max +/- change for moisture
  const float tempVariation = 0.3;     // Max +/- change for temperature
  const float humidityVariation = 0.5; // Max +/- change for humidity
  const int npkVariation = 1;          // Max +/- change for NPK values
  
  // Randomize moisture (realistic range based on dataset: 20-70%)
  moistureValue = prevMoistureValue + random(-moistureVariation, moistureVariation + 1);
  moistureValue = constrain(moistureValue, 20, 70);
  prevMoistureValue = moistureValue;
  
  // Randomize temperature (realistic range based on dataset: 20-40°C)
  temperature = prevTemperature + (random(-tempVariation * 10, tempVariation * 10 + 1) / 10.0);
  temperature = constrain(temperature, 20.0, 40.0);
  prevTemperature = temperature;
  
  // Randomize humidity (realistic range based on dataset: 40-80%)
  humidity = prevHumidity + (random(-humidityVariation * 10, humidityVariation * 10 + 1) / 10.0);
  humidity = constrain(humidity, 40.0, 80.0);
  prevHumidity = humidity;
  
  // Randomize NPK values with specific ranges for each nutrient based on dataset
  // K - Potassium (index 0)
  npkValues[0] = prevNpkValues[0] + random(-npkVariation, npkVariation + 1);
  npkValues[0] = constrain(npkValues[0], 0, 45);
  prevNpkValues[0] = npkValues[0];
  
  // N - Nitrogen (index 1)
  npkValues[1] = prevNpkValues[1] + random(-npkVariation, npkVariation + 1);
  npkValues[1] = constrain(npkValues[1], 0, 45);
  prevNpkValues[1] = npkValues[1];
  
  // P - Phosphorus (index 2)
  npkValues[2] = prevNpkValues[2] + random(-npkVariation, npkVariation + 1);
  npkValues[2] = constrain(npkValues[2], 0, 45);
  prevNpkValues[2] = npkValues[2];
  
  // Print randomized values to serial for debugging
  Serial.println("Randomized Moisture: " + String(moistureValue));
  Serial.println("Randomized Temperature: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");
  for (uint8_t i = 0; i < NPK_COMMANDS; i++) {
    Serial.println("Randomized " + String(npkLabels[i]) + String(npkValues[i]));
  }
}