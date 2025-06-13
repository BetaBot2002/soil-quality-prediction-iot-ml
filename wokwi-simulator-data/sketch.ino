#define BLYNK_TEMPLATE_ID "TMPL3grG9nPT7"
#define BLYNK_TEMPLATE_NAME "soil moisture and npk"
#define BLYNK_AUTH_TOKEN "gFxZ4mVCpGZFPmfDFYFFeOtDOb9_5hxY"

// Pin definitions
#define LED_PIN 26
#define BUZZER_PIN 27
#define DHTPIN 15     // DHT22 sensor pin
#define MOISTURE_PIN 34  // Analog pin for moisture sensor

// Include necessary libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>  // For HTTPS support
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"  // DHT sensor library
#include <HTTPClient.h>
#include <ArduinoJson.h>  // For JSON formatting

// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Server endpoint for sending sensor data
const char* serverUrl = "https://soil-quality-prediction-iot-ml.onrender.com/write_data";
const char* serverHost = "soil-quality-prediction-iot-ml.onrender.com";
const int serverPort = 443;  // HTTPS port

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

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");

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
 * Sends sensor data to the cloud server via HTTPS
 * Creates a JSON payload with all sensor readings and sends it to the server
 */
void sendDataToServer() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }
  
  // Create JSON document with sensor data
  StaticJsonDocument<256> doc;
  doc["moisture"] = moistureValue;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["nitrogen"] = npkValues[1];    // Nitrogen value
  doc["phosphorus"] = npkValues[2];  // Phosphorous value
  doc["potassium"] = npkValues[0];   // Potassium value
  
  // Serialize JSON to string
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Log connection attempt
  Serial.println("Connecting to server: " + String(serverUrl));
  Serial.println("Payload: " + jsonString);
  
  // Initialize secure client
  WiFiClientSecure client;
  client.setInsecure();  // Skip certificate verification (required for Wokwi)
  
  // Connect to server
  if (!client.connect(serverHost, serverPort)) {
    Serial.println("Connection failed!");
    return;
  }
  
  // Craft HTTP request
  String httpRequest = String("POST /write_data HTTP/1.1\r\n") +
                       "Host: " + serverHost + "\r\n" +
                       "Connection: close\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + jsonString.length() + "\r\n" +
                       "\r\n" + 
                       jsonString;
  
  // Send request
  client.print(httpRequest);
  client.flush();
  
  // Wait for response with timeout
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 15000) {  // 15 second timeout
      Serial.println("Request timeout!");
      client.stop();
      return;
    }
  }
  
  // Process response
  String response = "";
  while (client.available()) {
    char c = client.read();
    response += c;
  }
  
  // Extract and log response code
  int codePos = response.indexOf(" ") + 1;
  int codeEndPos = response.indexOf(" ", codePos);
  String responseCode = response.substring(codePos, codeEndPos);
  
  Serial.println("HTTP Response code: " + responseCode);
  
  // Close connection
  client.stop();
}

/**
 * Main loop function - runs repeatedly
 */
void loop() {
  Blynk.run();  // Process Blynk communications
  
  // Read moisture sensor
  moistureValue = analogRead(MOISTURE_PIN);
  Serial.println("Moisture: " + String(moistureValue));
  
  // Read temperature and humidity
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (dhtSensor.getStatus() != 0) {
    Serial.println("DHT sensor error: " + String(dhtSensor.getStatusString()));
    temperature = 0;
    humidity = 0;
  } else {
    temperature = data.temperature;
    humidity = data.humidity;
    Serial.println("Temperature: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");
  }

  // Read NPK sensor values
  for (uint8_t i = 0; i < NPK_COMMANDS; i++) {
    Serial2.print((char)npkCommands[i]);  // Send command to NPK sensor
    if (Serial2.available()) {
      npkValues[i] = Serial2.read();  // Read response
      Serial2.flush();  // Clear buffer
      Serial.println(npkLabels[i] + String(npkValues[i]));
    }
    delay(500);  // Short delay between commands
  }
  
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
  
  // Send data to cloud server
  sendDataToServer();
  
  // Wait before next reading
  delay(2000);
}
