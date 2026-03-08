#include <SPI.h>

const int CS = 10;

void setup() {
  Serial.begin(115200);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
}

uint8_t spiXfer(uint8_t b) {
  digitalWrite(CS, LOW);
  uint8_t r = SPI.transfer(b);
  digitalWrite(CS, HIGH);
  return r;
}

void loop() {
  uint8_t r = spiXfer(0x9F); 
  Serial.print("RX=");
  Serial.println(r, HEX);
  delay(1000);
}