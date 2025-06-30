#include <Bonezegei_DHT11.h>

#define DHTPIN 16

Bonezegei_DHT11 dht(DHTPIN);

void setup() {
  Serial.begin(115200);
  Serial.println("DHT11 Test with ESP32-S3 using Bonezegei_DHT11");
  
  dht.begin();
}

void loop() {
  if (dht.getData()) { // Attempt to read sensor data
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%  Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
  } else {
    Serial.println("Failed to read from DHT11 sensor!");
  }
  
  delay(2000); // Wait 2 seconds between measurements
}