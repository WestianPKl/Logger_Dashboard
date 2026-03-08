const int ADC_PIN = A0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int v = analogRead(ADC_PIN);
  Serial.println(v);
  delay(200);
}