#define BUZZER_PIN 5
void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER_PIN , OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(BUZZER_PIN , HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN , LOW);
  delay(1000);
}
