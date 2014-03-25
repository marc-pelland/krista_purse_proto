#include <Wire.h>  // I2C Wire Library for communicating with the DS1307 RTC
#include <OneWire.h>
#include <Adafruit_CharacterOLED.h>
#include <DallasTemperature.h>
#include "RTClib.h" // Date and time functions for the DS1307 RTC connected
//#include <LiquidCrystal.h> // Display functions for the LCD Display
RTC_DS1307 rtc;       // Create a realtime clock called rtc
#define DS1307_ADDRESS 0x68

#define ONE_WIRE_BUS 8

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Create an LCD called lcd
Adafruit_CharacterOLED lcd(OLED_V2, 13, 12, 11, 10, 9, 8, 7);

void setup () {
 Wire.begin();  // Enables the communication for the LCD
 rtc.begin();  // Enables the RTC
 lcd.begin(16, 2); // Enables the LCD
 // Start up the library
 Serial.begin(9600);
  sensors.begin();
 lcd.print(" It's a clock!"); // Print a message, centered, to the LCD to confirm it’s working
 delay(500);     // Wait a moment so we can read it
 lcd.clear();

}

void loop(){

  sensors.requestTemperatures();
lcd.setCursor(0,0);
lcd.print("time: ");
  printDate();
  lcd.setCursor(0,1);
  lcd.print(round(sensors.getTempCByIndex(0))); 
   lcd.print(" degrees C");
  delay(1000);
}


byte bcdToDec(byte val)  {
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

void printDate(){

  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);

  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());

  //print the date EG   3/1/11 23:59:59
//  Serial.print(month);
//  Serial.print("/");
//  Serial.print(monthDay);
//  Serial.print("/");
//  Serial.print(year);
//  Serial.print(" ");
if (hour >= 12) {
  if (minute < 10) {
    lcd.print(String(hour%12) + ":0" + String(minute) + " pm");
  } else {
    lcd.print(String(hour%12) + ":" + String(minute) + " pm");
  }
} else {
  if (minute < 10) {
    lcd.print(String(hour) + ":" + String(minute) + " am");
  } else {
    lcd.print(String(hour) + ":0" + String(minute) + " am");
  }
}
//  Serial.print(hour%12);
//  Serial.print(":");
//  Serial.print(minute);
//  Serial.print(":");
//  Serial.println(second);

  

}
