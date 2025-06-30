// LCD library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// DHT11 library
#include "Bonezegei_DHT11.h"
// RTC library
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define SDA_PIN 13
#define SCL_PIN 14
#define LCD_ADDRESS 0x27 // LCD I2C address
#define MQ7_PIN 17       // MQ-7 analog output on GPIO 17
#define DHT_PIN 16       // DHT11 data pin on GPIO 15
#define BUTTON_UP 18     // Up button on GPIO 18
#define BUTTON_DOWN 8    // Down button on GPIO 8
#define BUZZER_PIN 5     // Buzzer on GPIO 5
#define COLUMNS 16
#define ROWS 2

// MQ-7 sensor resistance in clean air (kOhms), calibrate in clean air
const float R0 = 27.5;

// Timing variables
unsigned long previousMillis = 0;
const unsigned long warmupTime = 3000; // 3 seconds for MQ-7 warm-up
const unsigned long displayInterval = 1000; // 1 seconds per display
const unsigned long buzzerDuration = 20; // 20ms buzzer on
const unsigned long dhtInterval = 1000; // 2 seconds for DHT11 reads
const unsigned long serialInterval = 1000; // 1 second for Serial prints
const unsigned long debounceTime = 300; // 300ms debounce for buttons

// Display and buzzer states
volatile int displayMode = 0; // 0: Time, 1: Temperature & Humidity, 2: CO ppb & Atmosphere quality
const int maxModes = 2;
volatile bool buttonUpPressed = false;
volatile bool buttonDownPressed = false;
unsigned long lastUpInterrupt = 0;
unsigned long lastDownInterrupt = 0;
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;

const char* ssid     = "Trongtin";
const char* password = "07072005";

const char* ntpServer = "vn.pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 25200;//GMT+5:30

// Sensor data
int mq7Value = 0;
float co_ppb = 0.0;
float temperature = 0.0;
float humidity = 0.0;
String tempStatus = "";
String humStatus = "";
String timeStatus = "";

// Initialize the LCD and DHT11
LiquidCrystal_I2C lcd(LCD_ADDRESS, COLUMNS, ROWS);
Bonezegei_DHT11 dht(DHT_PIN);

// Interrupt handlers for buttons
void IRAM_ATTR handleUpButton() {
  unsigned long currentMillis = millis();
  if(displayMode < 0)
  {
    displayMode = maxModes;
  }
  else if(displayMode > maxModes)
  {
    displayMode = 0;
  }
  // Debounce button using millis
  if (currentMillis - lastUpInterrupt > debounceTime) {
    buttonUpPressed = true;
    displayMode = (displayMode + 1) ;
    lastUpInterrupt = currentMillis;
  }
}

void IRAM_ATTR handleDownButton() {
  unsigned long currentMillis = millis();
  if(displayMode < 0)
  {
    displayMode = maxModes;
  }
  else if(displayMode > maxModes)
  {
    displayMode = 0;
  }
  // Debounce button using millis
  if (currentMillis - lastDownInterrupt > debounceTime) {
    buttonDownPressed = true;
    displayMode = displayMode - 1 ; // Previous mode
    lastDownInterrupt = currentMillis;
  }
}

void setup() {
  // Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize Serial
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for Serial
  }
  
  // Initialize DHT11
  dht.begin();
  
  // Initialize buttons with internal pull-ups
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Attach interrupts for buttons
  attachInterrupt(digitalPinToInterrupt(BUTTON_UP), handleUpButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_DOWN), handleDownButton, FALLING);
  
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
    //Internal RTC setup
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
    // Receive time from sever
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  // else
  // {
  //   // If fail to connect to sever , user set time manual
  // timeinfo.tm_year = 2025 - 1900; // Years since 1900
  // timeinfo.tm_mon = 6 - 1;        // Months are 0-11
  // timeinfo.tm_mday = 27;          // Day of the month
  // timeinfo.tm_hour = 14;          // Hours
  // timeinfo.tm_min = 7;            // Minutes
  // timeinfo.tm_sec = 0;            // Seconds
  // timeinfo.tm_isdst = -1;         // Auto-detect DST
  // }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Initialize RTC (June 27, 2025, 14:07:00 +07)
  // struct tm timeinfo;

  struct timeval tv;
  tv.tv_sec = mktime(&timeinfo);
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  
  // Display warmup message
  lcd.setCursor(0, 0);
  lcd.print("MQ-7 DHT11 RTC");
  lcd.setCursor(0, 1);
  lcd.print("Warming Up...");
  
  // Non-blocking warm-up for MQ-7
  unsigned long startMillis = millis();
  while (millis() - startMillis < warmupTime) {
    // Do nothing, just wait for 24 seconds
  }
  lcd.clear();
  
  Serial.println("System Started");
}

void loop() {
  unsigned long currentMillis = millis();


  // Handle buzzer timing
  if ((buttonUpPressed || buttonDownPressed) && !buzzerActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println(displayMode);
    delay(100);
    // delay(10);
    buzzerActive = true;
    buzzerStartTime = currentMillis;
  }
  if (buzzerActive && (currentMillis - buzzerStartTime >= buzzerDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
  }
  
  // Print button states to Serial
  if (buttonUpPressed) {
    Serial.println("Up Button Pressed");
    buttonUpPressed = false; // Reset flag
  }
  if (buttonDownPressed) {
    Serial.println("Down Button Pressed");
    buttonDownPressed = false; // Reset flag
  }
  
  // Read sensors periodically
  static unsigned long lastDhtRead = 0;
  if (currentMillis - lastDhtRead >= dhtInterval) {
    // Read MQ-7 sensor
    mq7Value = analogRead(MQ7_PIN);
    float mq7Voltage = mq7Value * (3.3 / 4095.0);
    float RS = ((3.3 / mq7Voltage) - 1) * 1000; // Sensor resistance
    float ratio = RS / R0;
    co_ppb = 1000 * pow(10, ((-1.518) * log10(ratio) - 0.685204)); // MQ-7 datasheet curve
    
    // Read DHT11 sensor
    if (dht.getData()) { // Read successful
      temperature = dht.getTemperature() ; // Convert to °C
      humidity = dht.getHumidity() ;       // Convert to %
      tempStatus = String(temperature, 1) + "C";
      humStatus = String(humidity, 0) + "%";
    } else {
      tempStatus = "Err";
      humStatus = "Err";
    }
    
    lastDhtRead = currentMillis;
  }
  
  // Update RTC time
  struct tm timeinfo;
  time_t now;
  time(&now);
  localtime_r(&now, &timeinfo);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  timeStatus = String(timeStr);
  
  // Update display periodically or on button press
  static unsigned long lastDisplayUpdate = 0;
  if (currentMillis - lastDisplayUpdate >= displayInterval || buttonUpPressed || buttonDownPressed) {
    lcd.clear();
    switch (displayMode)
    {
      case 0:
      
        // Display 1: Time
        lcd.setCursor(0, 0);
        lcd.print(timeStatus.substring(0, 10)); // YYYY-MM-DD
        lcd.setCursor(0, 1);
        lcd.print(timeStatus.substring(11));    // HH:MM:SS
      
      break;

      case 1: 
      
        // Display 2: Sensors (MQ-7 and DHT11)
        lcd.setCursor(0, 0);
        lcd.print("CO:");
        lcd.print(co_ppb, 2);
        lcd.print("ppb");
        lcd.setCursor(0, 1);
        lcd.print("Status: ");
        if(co_ppb < 0.15)
        {
          lcd.print("Normal");
        }
        else if (co_ppb < 0.4)
        {
          lcd.print("Danger");
        }
        else
        {
          lcd.print("Dead!");
        }

      break;

      case 2:
      
        lcd.setCursor(0, 0);
        lcd.print("Temp:");
        lcd.print(tempStatus);
        lcd.setCursor(0, 1);
        lcd.print("Humi:");
        lcd.print(humStatus);
      
      break;

      // // Reset displayMode
      // case -1:

      //   displayMode = maxModes;

      // break;

      // case 3:

      //   displayMode = 0;

      // break;  

      // Default mode
      default :
        lcd.setCursor(0, 0);
        lcd.print(timeStatus.substring(0, 10)); // YYYY-MM-DD
        lcd.setCursor(0, 1);
        lcd.print(timeStatus.substring(11));    // HH:MM:SS
      break;
    }
    lastDisplayUpdate = currentMillis;
  }

  printLocalTime();



  if(co_ppb > 0.4)
  {
    // Arlert in case very dangerous
    digitalWrite(BUZZER_PIN, HIGH);
  }

  
  // Print sensor data and time to Serial for debugging
  static unsigned long lastSerialPrint = 0;
  if (currentMillis - lastSerialPrint >= serialInterval) {
    Serial.print("MQ-7 Raw: ");
    Serial.print(mq7Value);
    Serial.print(" | CO: ");
    Serial.print(co_ppb);
    Serial.print(" ppb | Temp: ");
    Serial.print(tempStatus);
    Serial.print(" | Humidity: ");
    Serial.print(humStatus);
    Serial.print(" | Time: ");
    Serial.print(timeStatus);
    Serial.print(" | Mode: ");
    Serial.println(displayMode);
    lastSerialPrint = currentMillis;
  }

  if(displayMode < 0)
  {
    displayMode = maxModes;
  }
  else if(displayMode > maxModes)
  {
    displayMode = 0;
  }
}

// RTC function
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

}