#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi Parameters
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PWD";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  lcd.init();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Wire.begin(2,0);
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("  Chargement...");
  lcd.setCursor(0, 1);
  lcd.print("  Cedric JUNG");
  lcd.backlight(); 
  uint8_t eAigu[8] = {130,132,142,145,159,144,142,128};
  lcd.createChar(0, eAigu);
}

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

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  
    http.begin("http://example.com/api/weather-data/::ffff:192.168.0.15/displayer/");
    int httpCode = http.GET();                                                        
    if (httpCode > 0) {
      // Parsing
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      float temperature = root["temperature"]; 
      float humidity = root["humidity"]; 
      showTempAndHumidity(temperature, humidity);
    }
    http.end();
  }
  delay(600000);
}
