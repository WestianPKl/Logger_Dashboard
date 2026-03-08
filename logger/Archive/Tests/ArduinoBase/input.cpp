const int BTN = 2;
volatile bool fired = false;

void isrBtn() {
  fired = true;
}

void setup() {
  pinMode(BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN), isrBtn, FALLING);
  Serial.begin(115200);
}

void loop() {
  if (fired) {
    fired = false;
    Serial.println("Interrupt!");
  }
}