#define BLYNK_TEMPLATE_ID "TMPL3grG9nPT7"
#define BLYNK_TEMPLATE_NAME "soil moisture and npk"
#define BLYNK_AUTH_TOKEN "gFxZ4mVCpGZFPmfDFYFFeOtDOb9_5hxY"

#define ledpin 26
#define BUZZER_PIN 27
#define DHTPIN 15     // Changed to match the DHT folder example


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"  // Changed to DHTesp library

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
const int potPin = 34;
// variable for storing the potentiometer value
int potValue = 0;

#define ncom 3 // number of commands
char commar[ncom] = {0x1, 0x3, 0x5}; // Actual commands
// Response Strings can be stored like this
char respar[ncom][30] = {"Potassium value is: ", "Nitrogen value is: ", "Phosphorous value is: "};
uint8_t rtValue[ncom]; // Store the return values from the custom chip in here


LiquidCrystal_I2C lcd(0x27, 20, 4);
DHTesp dhtSensor; // Changed to DHTesp

// Variables for storing temperature and humidity
float temperature = 0;
float humidity = 0;


void beepBuzzer(int frequency, int duration, int repeat) {
  // Function to produce beeping sounds
  for (int i = 0; i < repeat; i++) {
    tone(BUZZER_PIN, frequency, duration);
    delay(300); // Delay between consecutive beeps
  }
  delay(2000); // Delay after the entire sequence
}


void setup() {
  Serial.begin(115200);
  Serial2.begin(15200, SERIAL_8N1, 16, 17); // Initialize the custom chip communication line

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Hello, ESP32!");

  pinMode(ledpin, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init();         // initialize the lcd
  lcd.backlight(); // Turn on the LCD screen backlight
  
  dhtSensor.setup(DHTPIN, DHTesp::DHT22);  // Changed to DHTesp setup method
  Serial.println("DHT22 sensor initialized");
  
  // Add a delay to allow the DHT sensor to stabilize
  delay(2000);
}

void loop() {
  Blynk.run(); // Run Blynk
  
  // Reading potentiometer value (moisture sensor)
  potValue = analogRead(potPin);
  Serial.println("Moisture: " + String(potValue));
  
  // Reading DHT22 sensor data with DHTesp library
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  
  // Check if any reads failed
  if (dhtSensor.getStatus() != 0) {
    Serial.println("Failed to read from DHT sensor: " + String(dhtSensor.getStatusString()));
    temperature = 0;
    humidity = 0;
  } else {
    temperature = data.temperature;
    humidity = data.humidity;
    
    Serial.print("Temp: ");
    Serial.print(temperature, 2);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println("%");
    
    // Send temperature and humidity data to Blynk
    Blynk.virtualWrite(V5, temperature);
    Blynk.virtualWrite(V6, humidity);
  }

  // Read NPK sensor values
  for (uint8_t i = 0; i < ncom; i++) {
    Serial2.print((char)commar[i]); // Send the command stored in commar array through serial2
    if (Serial2.available()) { // If serial2 data is there
      rtValue[i] = Serial2.read(); // Read serial2
      Serial2.flush(); // Flush serial2, very important to avoid extra bits interference
      Serial.print(respar[i]); // Print the response array to the console
      Serial.println(rtValue[i]); // Print the return value with newline at console
    }
    delay(1000); // Reduced delay for faster response
  }
  
  // Alert conditions - now including temperature and humidity thresholds
  if ((potValue < 20) && (rtValue[0] < 10 || rtValue[1] < 10 || rtValue[2] < 10) || 
      (temperature > 35.0) || (humidity < 30.0)){
    digitalWrite(ledpin, HIGH);
    beepBuzzer(3000, 1000, 3);
    Blynk.virtualWrite(V4, HIGH);
  } else {
    digitalWrite(ledpin, LOW);
    beepBuzzer(0,0,0);
    Blynk.virtualWrite(V4, LOW);
  }

  // Update LCD display with all sensor values
  lcd.clear(); // Clear the display before writing new values
  lcd.setCursor(0,0);
  lcd.print("Moist:");
  lcd.print(potValue);
  lcd.print(" T:");
  lcd.print(temperature, 1);
  lcd.print("C");
  
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 1);
  lcd.print("%");
  
  lcd.setCursor(0,2);
  lcd.print("N:");
  lcd.print(rtValue[1]);
  lcd.print(" P:");
  lcd.print(rtValue[2]);
  lcd.print(" K:");
  lcd.print(rtValue[0]);
  
  lcd.setCursor(0,3);
  if ((temperature > 35.0) || (humidity < 30.0)) {
    lcd.print("Climate Alert!");
  } else if (potValue < 20) {
    lcd.print("Moisture Alert!");
  } else if (rtValue[0] < 10 || rtValue[1] < 10 || rtValue[2] < 10) {
    lcd.print("Nutrient Alert!");
  } else {
    lcd.print("All Parameters OK");
  }

  Blynk.virtualWrite(V0, potValue); // Moisture value
  Blynk.virtualWrite(V1, rtValue[1]); // Nitrogen value
  Blynk.virtualWrite(V2, rtValue[2]); // Phosphorous value
  Blynk.virtualWrite(V3, rtValue[0]); // Potassium value

  Blynk.run();
  
  delay(2000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)
}
