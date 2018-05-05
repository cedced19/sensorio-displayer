#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Buttons Parameters
int buttonLightPin = 5;
int buttonShortLightPin = 12;
int buttonDetailPin = 4;

// DHT Parameters
#include "DHT.h"
#define DHTPIN 13
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// LCD Parameters
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi Parameters
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PWD";

// Timers
unsigned long timerExt = 0;
unsigned long timerIn = 0;
unsigned long timerSend = 0;

float temperatureOut;
float humidityOut;
float temperatureIn;
float humidityIn;

void showTempAndHumidity (float temperature, float humidity) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.setCursor(6, 0);
      lcd.print(temperature);
      lcd.setCursor(11, 0);
      lcd.print((char) 223);
      lcd.setCursor(12, 0);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Humidite:");
      lcd.setCursor(7, 1); 
      lcd.write(0);
      lcd.setCursor(10, 1);
      lcd.print(humidity);
      lcd.setCursor(15, 1);
      lcd.print("%");
}


void showOutsideAndInside (float outside, float inside) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ext:");
      lcd.setCursor(6, 0);
      lcd.print(outside);
      lcd.setCursor(11, 0);
      lcd.print((char) 223);
      lcd.setCursor(12, 0);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Int:");
      lcd.setCursor(6, 1);
      lcd.print(inside);
      lcd.setCursor(11, 1);
      lcd.print((char) 223);
      lcd.setCursor(12, 1);
      lcd.print("C");
}

void updateDataExt () {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  
    http.begin("http://example.com:8888/api/weather-data/::ffff:192.168.0.15/displayer/");
    int httpCode = http.GET();                                                        
    if (httpCode > 0) {
      // Parsing
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      temperatureOut = root["temperature"]; 
      humidityOut = root["humidity"]; 
      lcd.setCursor(6, 0);
      lcd.print(temperatureOut);
    }
    http.end();
  }
}

void sendDataSensor (float temperature, float humidity) {
    if (temperature > 0 && humidity > 0) {
          float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
          char params[100];
          char hB[10];
          dtostrf(humidity, 4, 2, hB);
          char tB[10];
          dtostrf(temperature, 4, 2, tB);
          
          char hicB[10];
          dtostrf(heatIndex, 4, 2, hicB);
    
          snprintf(params, 100, "humidity=%s&temperature=%s&heat_index=%s", hB, tB, hicB);
          
          HTTPClient http;
          http.begin("http://example.com:8888/api/weather-data/");
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          http.POST(params);    
          http.end();
    }
}

void updateDataIn () {
      float tmpT = dht.readTemperature();
      float tmpH = dht.readHumidity();
      if (!isnan(tmpT) || !isnan(tmpH)) {
        temperatureIn = tmpT;
        humidityIn = tmpH;
        lcd.setCursor(6, 1);
        lcd.print(temperatureIn);
      }
}

bool light = true;
void toogleLight () {
  if (light == true) {
      lcd.noBacklight();
  } else {
      lcd.backlight();
  }
  light=!light;
  delay(500);
}


void showDetail () {
  if (light == false) {
    lcd.backlight();
    delay(2000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Exterieur");
  lcd.setCursor(6, 0); 
  lcd.write(0);
  delay(1000);
  showTempAndHumidity(temperatureOut, humidityOut);
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Interieur");
  lcd.setCursor(6, 0); 
  lcd.write(0);
  delay(1000);
  showTempAndHumidity(temperatureIn, humidityIn);
  delay(4000);
  showOutsideAndInside(temperatureOut, temperatureIn);
  delay(3000);
  if (light == false) {
    lcd.noBacklight();
  }
}

void setup() {
  // LCD
  Wire.begin(2,0);
  lcd.init();
  // Special characters
  uint8_t eAigu[8] = {130,132,142,145,159,144,142,128};
  lcd.createChar(0, eAigu);
  lcd.backlight(); 
  lcd.setCursor(0, 0);
  lcd.print("  Chargement...");
  lcd.setCursor(0, 1);
  lcd.print("  Cedric JUNG");
  lcd.setCursor(3, 1); 
  lcd.write(0);
  // DHT11
  dht.begin();
  // Button
  pinMode(buttonLightPin, INPUT);
  // WiFi
  WiFi.begin(ssid, password);
  // Check for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  // Load data
  showOutsideAndInside(0,0);
  updateDataExt();
  updateDataIn();
  Serial.begin(115200);
}

void loop() {
   if(millis() > timerExt + 600000){
        timerExt = millis();
        updateDataExt();
   }
   if(millis() > timerIn + 60000){
        timerIn = millis();
        updateDataIn();
   }
   if(millis() > timerSend + 600000) {
         timerSend = millis();
         sendDataSensor(temperatureIn, humidityIn);
   }
   if (digitalRead(buttonLightPin) == HIGH) {
         toogleLight();
   }
   if (digitalRead(buttonShortLightPin) == HIGH) {
         if (light == false) {
            lcd.backlight();
            delay(5000);
            lcd.noBacklight();
         }
   }
   if (digitalRead(buttonDetailPin) == HIGH) {
         showDetail();
   }
}
