#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(200);
  Serial.println("I2C scan...");
}

void loop() {
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("Found 0x");
      Serial.println(addr, HEX);
    }
  }
  delay(3000);
}