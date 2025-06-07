#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ModbusMaster.h>
#include <ArduinoJson.h>

// Pin Definitions
#define DHT_PIN 4
#define SOIL_MOISTURE_PIN 36
#define SOIL_TYPE_PIN 39
#define NPK_RX 16
#define NPK_TX 17

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://your-server-ip:5000/sensor-data";

// Sensor objects
DHT dht(DHT_PIN, DHT22);
ModbusMaster node;

// Variables for sensor readings
float temperature = 0;
float humidity = 0;
float soilMoisture = 0;
float soilType = 0;
int nitrogen = 0;
int phosphorous = 0;
int potassium = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize NPK sensor
  Serial2.begin(9600, SERIAL_8N1, NPK_RX, NPK_TX);
  node.begin(1, Serial2);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void readDHT() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void readSoilMoisture() {
  int rawValue = analogRead(SOIL_MOISTURE_PIN);
  soilMoisture = map(rawValue, 4095, 0, 0, 100); // Convert to percentage
}

void readSoilType() {
  int rawValue = analogRead(SOIL_TYPE_PIN);
  // Simple mapping of conductivity to soil type
  // This is a basic implementation and should be calibrated
  if (rawValue < 1000) soilType = 0;      // Sandy
  else if (rawValue < 2000) soilType = 1;  // Loamy
  else if (rawValue < 3000) soilType = 2;  // Red
  else soilType = 3;                       // Clayey
}

void readNPK() {
  uint8_t result = node.readHoldingRegisters(0, 3);
  if (result == node.ku8MBSuccess) {
    nitrogen = node.getResponseBuffer(0);
    phosphorous = node.getResponseBuffer(1);
    potassium = node.getResponseBuffer(2);
  } else {
    Serial.println("Failed to read NPK sensor!");
  }
}

void sendData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON document
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["moisture"] = soilMoisture;
    doc["soil_type"] = soilType;
    doc["nitrogen"] = nitrogen;
    doc["phosphorous"] = phosphorous;
    doc["potassium"] = potassium;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Send POST request
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
    
    http.end();
  }
}

void loop() {
  // Read all sensors
  readDHT();
  readSoilMoisture();
  readSoilType();
  readNPK();
  
  // Print readings
  Serial.println("\nSensor Readings:");
  Serial.println("Temperature: " + String(temperature) + "°C");
  Serial.println("Humidity: " + String(humidity) + "%");
  Serial.println("Soil Moisture: " + String(soilMoisture) + "%");
  Serial.println("Soil Type: " + String(soilType));
  Serial.println("Nitrogen: " + String(nitrogen) + " mg/kg");
  Serial.println("Phosphorous: " + String(phosphorous) + " mg/kg");
  Serial.println("Potassium: " + String(potassium) + " mg/kg");
  
  // Send data to server
  sendData();
  
  // Wait for 5 minutes before next reading
  delay(300000);
} 