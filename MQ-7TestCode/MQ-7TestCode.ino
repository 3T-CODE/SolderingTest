
#define MQ7_PIN 17

void setup() {
  
  Serial.begin(115200);
  
  
  pinMode(MQ7_PIN, INPUT);
  
  
  delay(1000);
  Serial.println("MQ-7 Sensor Test");
}

void loop() {
  
  int sensorValue = analogRead(MQ7_PIN);
  
  
  Serial.print("MQ-7 Sensor Value: ");
  Serial.println(sensorValue);
  
  
  delay(1000);
}