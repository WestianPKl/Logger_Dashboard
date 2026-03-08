void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hello UART");
  delay(1000);

  if (Serial.available()) {
    int b = Serial.read();
    Serial.print("RX: ");
    Serial.println(b);
  }
}