#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


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
      
      //showOutsideAndInside(temperatureOut, temperatureIn);
      //showTempAndHumidity(temperatureOut, humidityOut);
    }
    http.end();
  }
}

void updateDataIn () {
      float tmpT = dht.readTemperature();
      float tmpH = dht.readHumidity();
      if (!isnan(tmpT) || !isnan(tmpH)) {
        temperatureIn = tmpT;
        humidityIn = tmpH;
        float heatIndex = dht.computeHeatIndex(temperatureIn, humidityIn, false);
        lcd.setCursor(6, 1);
        lcd.print(temperatureIn);
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
  // DHT11
  dht.begin();
  // WiFi
  WiFi.begin(ssid, password);
  // Check for connection
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("  Connection...");
    delay(1000);
  }
  // Load data
  showOutsideAndInside(0,0);
  updateDataExt();
  updateDataIn();
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
}
