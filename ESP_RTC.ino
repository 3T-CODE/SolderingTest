#include <WiFi.h>
#include "time.h"

const char* ssid     = "PIF_CLUB";
const char* password = "Chinsochin";

const char* ntpServer = "vn.pool.ntp.org";
const long  gmtOffset_sec = 25200; // Thiet lap gio VN 7*60*60
const int   daylightOffset_sec = 0;

void setup(){
  Serial.begin(115200);

  // Ket noi wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Lay thoi gian tu sever
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

}
void loop(){
  delay(1000);
  printLocalTime();
}
