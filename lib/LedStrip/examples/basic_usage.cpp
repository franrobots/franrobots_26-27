#include <Arduino.h>
#include "LedStrip.h"

LedStrip leds(11, 5);

void setup() {
  leds.begin();
  leds.setColor(LedSide::ALL, LedColor::GREEN);
  delay(500);
  leds.blink(LedSide::LEFT, LedColor::YELLOW, 3, 250);
}

void loop() {
  leds.update();
}
