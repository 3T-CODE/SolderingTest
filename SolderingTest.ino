#include <stdint.h>
#include <WiFi.h>
#include "time.h"

#define   rowDataPin        5
#define   columnDataM1Pin   13
#define   columnDataM2Pin   12
#define   clock1Pin         2
#define   clock2Pin         4
#define   latchPin          23
#define   buttonUpPin       14
#define   buttonDownPin     27

const char* ssid     = "PIF_CLUB";
const char* password = "chinsochin";

const char* ntpServer = "vn.pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 19800;//GMT+5:30

int currentNumber = 0;
int tens = 0;
int units = 0;


// Volatile variables for interrupt handling and debouncing
volatile unsigned long lastInterruptTimeUp = 0;
volatile unsigned long lastInterruptTimeDown = 0;
const unsigned long debounceDelay = 200; // 200ms debounce delay

// Manual clock variable
volatile uint8_t modeFlag = 0;
volatile uint8_t modeCurrentState = 0;

unsigned int rowData[8] = 
{
  0b0000000000000001 ,
  0b0000000000000010 ,
  0b0000000000000100 ,
  0b0000000000001000 ,
  0b0000000000010000 ,
  0b0000000000100000 ,
  0b0000000001000000 ,
  0b0000000010000000 
  // 0b0000000100000000 ,
  // 0b0000001000000000 ,
  // 0b0000010000000000 ,
  // 0b0000100000000000 ,
  // 0b0001000000000000 ,
  // 0b0010000000000000 ,
  // 0b0100000000000000 ,
  // 0b1000000000000000
  };

const uint8_t numbers[10][8] = {
    // Number '0'
    {0b00000000, 0b00000000, 0b00010000, 0b00101000, 0b00101000, 0b00101000, 0b00010000, 0b00000000},
    // Number '1'
    {0b00000000, 0b00000000, 0b00010000, 0b00110000, 0b00010000, 0b00010000, 0b00010000, 0b00000000},
    // Number '2'
    {0b00000000, 0b00000000, 0b00111000, 0b00001000, 0b00010000, 0b00100000, 0b00111000, 0b00000000},
    // Number '3'
    {0b00000000, 0b00000000, 0b00111000, 0b00001000, 0b00011000, 0b00001000, 0b00111000, 0b00000000},
    // Number '4'
    {0b00000000, 0b00000000, 0b00101000, 0b00101000, 0b00111000, 0b00001000, 0b00001000, 0b00000000},
    // Number '5'
    {0b00000000, 0b00000000, 0b00111000, 0b00100000, 0b00111000, 0b00000100, 0b00111000, 0b00000000},
    // Number '6'
    {0b00000000, 0b00000000, 0b00111000, 0b00100000, 0b00111000, 0b00100100, 0b00111000, 0b00000000},
    // Number '7'
    {0b00000000, 0b00000000, 0b00111000, 0b00001000, 0b00010000, 0b00010000, 0b00010000, 0b00000000},
    // Number '8'
    {0b00000000, 0b00000000, 0b00111000, 0b00101000, 0b00111000, 0b00101000, 0b00111000, 0b00000000},
    // Number '9'
    {0b00000000, 0b00000000, 0b00111000, 0b00101000, 0b00111000, 0b00001000, 0b00111000, 0b00000000}
};

const uint8_t narrowNumbers[10][8] = {
  // 0
  {0b0000, 0b0110, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110, 0b0000},
  // 1
  {0b0010, 0b0110, 0b0010, 0b0010, 0b0010, 0b0010, 0b0111, 0b0000},
  // 2
  {0b0110, 0b1001, 0b0001, 0b0010, 0b0100, 0b1000, 0b1111, 0b0000},
  // 3
  {0b0110, 0b1001, 0b0001, 0b0110, 0b0001, 0b1001, 0b0110, 0b0000},
  // 4
  {0b0011, 0b0101, 0b1001, 0b1111, 0b0001, 0b0001, 0b0001, 0b0000},
  // 5
  {0b1111, 0b1000, 0b1110, 0b0001, 0b0001, 0b1001, 0b0110, 0b0000},
  // 6
  {0b0110, 0b1000, 0b1000, 0b1110, 0b1001, 0b1001, 0b0110, 0b0000},
  // 7
  {0b1111, 0b0001, 0b0010, 0b0100, 0b1000, 0b1000, 0b1000, 0b0000},
  // 8
  {0b0110, 0b1001, 0b1001, 0b0110, 0b1001, 0b1001, 0b0110, 0b0000},
  // 9
  {0b0110, 0b1001, 0b1001, 0b0111, 0b0001, 0b0001, 0b0110, 0b0000}
};

const uint8_t alphabet[26][8] = 
{
    // Letter 'a'
    {0b00000000, 0b00000000, 0b00111000, 0b01000100, 0b01111100, 0b01000100, 0b01000100, 0b00000000},
    // Letter 'b'
    {0b00000000, 0b01000000, 0b01111000, 0b01000100, 0b01111100, 0b01000100, 0b01111000, 0b00000000},
    // Letter 'c'
    {0b00000000, 0b00000000, 0b00111000, 0b01000000, 0b01000000, 0b01000000, 0b00111000, 0b00000000},
    // Letter 'd'
    {0b00000000, 0b00000100, 0b00111100, 0b01000100, 0b01111100, 0b01000100, 0b00111100, 0b00000000},
    // Letter 'e'
    {0b00000000, 0b00000000, 0b00111000, 0b01000100, 0b01111100, 0b01000000, 0b00111000, 0b00000000},
    // Letter 'f'
    {0b00000000, 0b00011000, 0b00100100, 0b01111100, 0b00100100, 0b00100100, 0b00100100, 0b00000000},
    // Letter 'g'
    {0b00000000, 0b00000000, 0b00111000, 0b01000100, 0b01111100, 0b01000100, 0b00111000, 0b01000100},
    // Letter 'h'
    {0b00000000, 0b01000000, 0b01111000, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b00000000},
    // Letter 'i'
    {0b00000000, 0b00010000, 0b00000000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00000000},
    // Letter 'j'
    {0b00000000, 0b00001000, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b01001000, 0b00110000},
    // Letter 'k'
    {0b00000000, 0b01000000, 0b01001100, 0b01011000, 0b01100100, 0b01000100, 0b01000100, 0b00000000},
    // Letter 'l'
    {0b00000000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00111000, 0b00000000},
    // Letter 'm'
    {0b00000000, 0b00000000, 0b01101100, 0b01010100, 0b01010100, 0b01010100, 0b01010100, 0b00000000},
    // Letter 'n'
    {0b00000000, 0b00000000, 0b01111000, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b00000000},
    // Letter 'o'
    {0b00000000, 0b00000000, 0b00111000, 0b01000100, 0b01111100, 0b01000100, 0b00111000, 0b00000000},
    // Letter 'p'
    {0b00000000, 0b00000000, 0b01111000, 0b01000100, 0b01111100, 0b01000000, 0b01000000, 0b01000000},
    // Letter 'q'
    {0b00000000, 0b00000000, 0b00111100, 0b01000100, 0b01111100, 0b00000100, 0b00000100, 0b00000100},
    // Letter 'r'
    {0b00000000, 0b00000000, 0b01111000, 0b01000100, 0b01000000, 0b01000000, 0b01000000, 0b00000000},
    // Letter 's'
    {0b00000000, 0b00000000, 0b00111100, 0b01000000, 0b00111000, 0b00000100, 0b01111000, 0b00000000},
    // Letter 't'
    {0b00000000, 0b00010000, 0b01111100, 0b00010000, 0b00010000, 0b00010000, 0b00011000, 0b00000000},
    // Letter 'u'
    {0b00000000, 0b00000000, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b00111000, 0b00000000},
    // Letter 'v'
    {0b00000000, 0b00000000, 0b01000100, 0b01000100, 0b01000100, 0b00101000, 0b00010000, 0b00000000},
    // Letter 'w'
    {0b00000000, 0b00000000, 0b01010100, 0b01010100, 0b01010100, 0b01010100, 0b00101000, 0b00000000},
    // Letter 'x'
    {0b00000000, 0b00000000, 0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100, 0b00000000},
    // Letter 'y'
    {0b00000000, 0b00000000, 0b01000100, 0b01000100, 0b00111100, 0b00000100, 0b00111000, 0b00000000},
    // Letter 'z'
    {0b00000000, 0b00000000, 0b01111100, 0b00000100, 0b00111000, 0b01000000, 0b01111100, 0b00000000}
};

// User functions declare 
void DisplayMatrix1(unsigned int Data);
int ColumnDataHandle(unsigned int columnData , int count);
void AlphabetShifting();
void NumberShifting();
void printNumber();
void printLocalTime();
void printCharacter();
void DisplayMatrix2(int rowData , int columnData);
void printCharacterM2(char c);
void ClockMode(int Mode , const struct tm*timeinfo);
void ClockModeAuto(const struct tm*timeinfo);
void IRAM_ATTR buttonUpISR() ;
void IRAM_ATTR buttonDownISR() ;

void setup() {
  // put your setup code here, to run once:
  //74hc595 setup
  
  pinMode(rowDataPin,OUTPUT);
  pinMode(columnDataM1Pin,OUTPUT);
  pinMode(columnDataM2Pin,OUTPUT);
  pinMode(clock1Pin,OUTPUT);
  pinMode(clock2Pin,OUTPUT);
  pinMode(latchPin,OUTPUT);

  //Button sellect pin setup
  pinMode(buttonDownPin,INPUT);
  pinMode(buttonUpPin,INPUT);
  // Attach interrupts to buttons
  attachInterrupt(digitalPinToInterrupt(buttonUpPin), buttonUpISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonDownPin), buttonDownISR, FALLING);

  //Internal RTC setup
  Serial.begin(115200);
  Serial.println("Enter a number (0-99):");
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
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  


  // printNumber(99);
  
  // printNumber(timeinfo.tm_sec);
  // printCharacterM2('s');
  
  ClockModeAuto(&timeinfo);


  // ClockModeManual(&timeinfo , 1);

  if(modeCurrentState > 5)
  {
    modeCurrentState = 0;
  }
  else if(modeCurrentState < 0)
  {
    modeCurrentState = 5;
  }

  // ClockMode(5,&timeinfo);
  // printLocalTime();

}


//User functions

void ClockMode(int Mode , const struct tm*timeinfo)
{
  int year = timeinfo->tm_year + 1900; // Year (e.g., 2025)
  int month = timeinfo->tm_mon + 1;    // Month (1–12)
  int day = timeinfo->tm_mday;         // Day (1–31)
  int hour = timeinfo->tm_hour;        // Hour (0–23)
  int min= timeinfo->tm_min;       // Minute (0–59)
  int sec = timeinfo->tm_sec;          // Second (0–59)
  if( (Mode >=0) && (Mode <= 5) )
  {
    switch (Mode) 
    {
      case 0 :
        printNumber(year%100);
        printCharacterM2('y'); 
        break;
      case 1 :  
        printNumber(month);
        printCharacterM2('m'); 
        break;
      case 2 :
        printNumber(day);
        printCharacterM2('d'); 
        break;
      case 3 :
        printNumber(hour);
        printCharacterM2('h'); 
        break;
      case 4 :
        printNumber(min);
        printCharacterM2('m'); 
        break;
      case 5 :
        printNumber(sec);
        printCharacterM2('s'); 
        break;
    }
  }
  else Mode == 0 ;
}

void ClockModeAuto(const struct tm*timeinfo)
{

  for (modeCurrentState = 0  ; modeCurrentState <=5 ; modeCurrentState++)
  {
    unsigned long timenow = millis();
    while ( (millis() - timenow) < 1000)
    {
      ClockMode(modeCurrentState , timeinfo);
      // Serial.println(modeFlag);
      if(digitalRead(buttonUpPin)||digitalRead(buttonDownPin) )
      {
        ClockModeManual(timeinfo , modeCurrentState);
      }
    }
  }
  

}

void ClockModeManual(const struct tm*timeinfo , int currentState)
{

  unsigned long timenow = millis();
  while(1)
  {
     
    if(currentState > 5)
    {
      currentState = 0;
    }
    else if(currentState < 0)
    {
      currentState = 5;
    }

    if ( ((millis() - timenow) >= 5000) ) 
    {
      Serial.println("It break !");
      
      break; 
    }
    else
    {
      
      Serial.println((millis() - timenow));
      Serial.println(!( (digitalRead(buttonUpPin) || digitalRead(buttonDownPin)) ));
      Serial.println("It continue !");
      Serial.println(currentState);
      ClockMode(currentState , timeinfo);
      if ( ( (digitalRead(buttonUpPin) || digitalRead(buttonDownPin)) ) )
      {
        timenow = millis();
        
        if(currentState > 5)
        {
          currentState = 0;
        }
        else if(currentState < 0)
        {
          currentState = 5;
        }
        else if((digitalRead(buttonUpPin)))
        {
          currentState ++ ;
          delay(200);
          
        }
        else if(digitalRead(buttonDownPin))
        {
          currentState -- ;
          delay(200);
        }
        else
        {
          currentState ++ ;
          delay(200);
        }
      }
    }
  }
  
}

char ButtonReadState(int buttonPin1 , int buttonPin2)
{
    if (buttonPin1 && buttonPin2 )
    {
      return 's'; // select
    }
    else if (buttonPin1)
    {
      return 'u'; // Up
    }
    else if (buttonPin2)
    {
      return 'd'; // Down
    }
    else
    {
      return 0; //Nothing
    }
}

void DisplayMatrix1(int rowData , int columnData)
{
  int columnPinState = 0;
  int rowPinState = 0;
        digitalWrite(latchPin,LOW);
      for(int i = 8 ; i >= 0 ; i--)  
      {
        
          digitalWrite(clock1Pin,LOW);
          digitalWrite(clock2Pin,LOW);
          if(rowData & (1<<i))
          {
            rowPinState = 1;
            columnPinState = ColumnDataHandle(columnData , i);
          }
          else
          {
            rowPinState = 0;
            columnPinState = ColumnDataHandle(columnData , i);
          }
          
          digitalWrite(rowDataPin,rowPinState);
          digitalWrite(columnDataM1Pin,columnPinState);
          digitalWrite(clock1Pin,HIGH);
          digitalWrite(clock2Pin,HIGH);

      }
      digitalWrite(latchPin,HIGH);

}


//User functions
void DisplayMatrix2(int rowData , int columnData)
{
  int columnPinState = 0;
  int rowPinState = 0;
        digitalWrite(latchPin,LOW);
      for(int i = 8 ; i >= 0 ; i--)  
      {
        
          digitalWrite(clock1Pin,LOW);
          digitalWrite(clock2Pin,LOW);
          if(rowData & (1<<i))
          {
            rowPinState = 1;
            columnPinState = ColumnDataHandle(columnData , i);
          }
          else
          {
            rowPinState = 0;
            columnPinState = ColumnDataHandle(columnData , i);
          }
          
          digitalWrite(rowDataPin,rowPinState);
          digitalWrite(columnDataM2Pin,columnPinState);
          digitalWrite(clock1Pin,HIGH);
          digitalWrite(clock2Pin,HIGH);

      }
      digitalWrite(latchPin,HIGH);

}


int ColumnDataHandle(int columnData , int count)
{
  int columnPinState = 0;
    if((~columnData) & (1<<count))
  {
    columnPinState = 1;
  }
  else
  {
    columnPinState = 0;
  }
  return columnPinState ;
}

void AlphabetShifting()
{
  // Buffer to hold combined pattern (current + next letter)
  uint8_t buffer[8][16];
  const int transitionSteps = 8; // Number of shifts to move one letter out
  const int stepDelay = 1000 / transitionSteps; // Delay per shift step (ms)

  // Cycle through letters (a-z)
  for (int currentLetter = 0; currentLetter < 26; currentLetter++) {
    int nextLetter = (currentLetter + 1) % 26; // Wrap around to 'a' after 'z'

    // Initialize buffer: current letter in columns 0-7, next letter in columns 8-15
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        buffer[row][col] = (alphabet[currentLetter][row] & (1 << (7 - col))) ? 1 : 0;
      }
      for (int col = 8; col < 16; col++) {
        buffer[row][col] = (alphabet[nextLetter][row] & (1 << (7 - (col - 8)))) ? 1 : 0;
      }
    }

    // Perform scrolling transition
    for (int shift = 0; shift <= transitionSteps; shift++) {
      unsigned long startTime = millis();
      while (millis() - startTime < stepDelay) {
        // Display the current 8x8 portion of the buffer
        for (int row = 0; row < 8; row++) {
          uint8_t columnData = 0;
          // Extract 8 columns from buffer, starting at 'shift'
          for (int col = 0; col < 8; col++) {
            if (buffer[row][col + shift]) {
              columnData |= (1 << (7 - col));
            }
          }
          DisplayMatrix1(rowData[row], columnData);
          delay(2); 
        }
      }
    }
  }
}

void NumberDisplay() {
  // Display each number (0-9) for 1 second
  for (int number = 0; number < 10; number++) {
    unsigned long startTime = millis();
    while (millis() - startTime < 1000) {
      // Scan through all rows to display the number
      for (int row = 0; row < 8; row++) {
        DisplayMatrix1(rowData[row], numbers[number][row]);
        delay(2); // Short delay for multiplexing
      }
    }
  }
}

void NumberShifting() {
  // Buffer to hold combined pattern (current + next number)
  uint8_t buffer[8][16];
  const int transitionSteps = 8; // Number of shifts to move one number out
  const int stepDelay = 1000 / transitionSteps; // Delay per shift step (ms)

  // Cycle through numbers (0-9)
  for (int currentNumber = 0; currentNumber < 10; currentNumber++) {
    int nextNumber = (currentNumber + 1) % 10; // Wrap around to '0' after '9'

    // Initialize buffer: current number in columns 0-7, next number in columns 8-15
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        buffer[row][col] = (numbers[currentNumber][row] & (1 << (7 - col))) ? 1 : 0;
      }
      for (int col = 8; col < 16; col++) {
        buffer[row][col] = (numbers[nextNumber][row] & (1 << (7 - (col - 8)))) ? 1 : 0;
      }
    }

    // Perform scrolling transition
    for (int shift = 0; shift <= transitionSteps; shift++) {
      unsigned long startTime = millis();
      while (millis() - startTime < stepDelay) {
        // Display the current 8x8 portion of the buffer
        for (int row = 0; row < 8; row++) {
          uint8_t columnData = 0;
          // Extract 8 columns from buffer, starting at 'shift' for left scroll
          for (int col = 0; col < 8; col++) {
            if (buffer[row][col + shift]) {
              columnData |= (1 << (7 - col));
            }
          }
          DisplayMatrix1(rowData[row], columnData);
          delay(2); // Short delay for multiplexing
        }
      }
    }
  }
}

void printNumber(int num) {
  int tens = 0;
  int units = 0;
  
  // Validate and split the number into tens and units
  if (num >= 0 && num <= 99) {
    tens = num / 10;
    units = num % 10;
  }

  // Display the number on the LED matrix
  for (int row = 0; row < 8; row++) {
    uint8_t tensPattern = narrowNumbers[tens][row];
    uint8_t unitsPattern = narrowNumbers[units][row];
    uint8_t columnData = (tensPattern << 4) | unitsPattern;
    DisplayMatrix1(rowData[row], columnData);
    delayMicroseconds(1000); // 1ms per row for refresh
  }
}

  //Display the character on the LED matrix
void printCharacter(char c) {
    if (c >= 'a' && c <= 'z') {
        int index = c - 'a';
        for (int row = 0; row < 8; row++) {
            uint8_t columnData = alphabet[index][row];
            DisplayMatrix1(rowData[row], columnData);
            delayMicroseconds(1000); // 1ms per row for refresh
        }
    }
}

void printCharacterM2(char c) {
    if (c >= 'a' && c <= 'z') {
        int index = c - 'a';
        for (int row = 0; row < 8; row++) {
            uint8_t columnData = alphabet[index][row];
            DisplayMatrix2(rowData[row], columnData);
            delayMicroseconds(1000); // 1ms per row for refresh
        }
    }
}

//Print time to Serial Monitor
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

}

// User Interrupt function
// Interrupt Service Routines (ISRs)
void IRAM_ATTR buttonUpISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTimeUp > debounceDelay) {
    lastInterruptTimeUp = currentTime;
    Serial.println("Up button pressed");
    modeFlag = 1;
    
  }
}

void IRAM_ATTR buttonDownISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTimeDown > debounceDelay) {
    lastInterruptTimeDown = currentTime;
    Serial.println("Down button pressed");
    modeFlag = 1;
  }
}
