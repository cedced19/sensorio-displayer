#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
void setup()
{
 lcd.init();
}
void loop()
{
 lcd.backlight();
 lcd.setCursor(0, 0);
 lcd.print("  Afficheur");
 lcd.setCursor(0,1);
 lcd.print(" Cedric JUNG");
}
