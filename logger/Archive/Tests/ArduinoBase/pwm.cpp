const int PWM_PIN = 9;

void setup() {
  pinMode(PWM_PIN, OUTPUT);
}

void loop() {
  analogWrite(PWM_PIN, 0);    // 0..255
  delay(1000);
  analogWrite(PWM_PIN, 128);
  delay(1000);
  analogWrite(PWM_PIN, 255);
  delay(1000);
}