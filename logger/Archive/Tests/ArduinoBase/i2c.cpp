#include <Wire.h>

#define SHT30_ADDR 0x44 

bool sht30_read(float &temperature, float &humidity) {
  Wire.beginTransmission(SHT30_ADDR);
  Wire.write(0x2C);
  Wire.write(0x06);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  delay(15);

  Wire.requestFrom(SHT30_ADDR, (uint8_t)6);
  if (Wire.available() != 6) {
    return false;
  }

  uint16_t t_raw = (Wire.read() << 8) | Wire.read();
  Wire.read();

  uint16_t rh_raw = (Wire.read() << 8) | Wire.read();
  Wire.read();
-
  temperature = -45.0 + 175.0 * ((float)t_raw / 65535.0);
  humidity    = 100.0 * ((float)rh_raw / 65535.0);

  return true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(200);

  Serial.println("SHT30 I2C demo");
}

void loop() {
  float t, rh;

  if (sht30_read(t, rh)) {
    Serial.print("Temp: ");
    Serial.print(t, 2);
    Serial.print(" C  |  RH: ");
    Serial.print(rh, 1);
    Serial.println(" %");
  } else {
    Serial.println("SHT30 read error");
  }

  delay(1000);
}