#define BUTTON_UP 18
#define BUTTON_DOWN 8

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(BUTTON_UP , INPUT);
  pinMode(BUTTON_DOWN , INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("button up state : ");
  Serial.print(digitalRead(BUTTON_UP));
  Serial.print(" button down state : ");
  Serial.println(digitalRead(BUTTON_DOWN));
  delay(200);
}
