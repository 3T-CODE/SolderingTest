#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SDA_PIN 13
#define SCL_PIN 14
// Initialize LCD with I2C address 0x27 (common for PCF8574), 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Initialize I2C communication
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight(); // Turn on backlight
}

void loop() {
  // Slide 1
  lcd.clear();
  lcd.setCursor(0, 0); // First line
  lcd.print("HELLO PIF KID");
  lcd.setCursor(0, 1); // Second line
  lcd.print("2025");
  delay(2000); // Display for 2 seconds

  // Slide 2
  lcd.clear();
  lcd.setCursor(0, 0); // First line
  lcd.print("Soldering");
  lcd.setCursor(0, 1); // Second line
  lcd.print("Course");
  delay(2000); // Display for 2 seconds
}