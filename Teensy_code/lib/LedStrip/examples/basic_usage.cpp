#include <Arduino.h>
#include "LedStrip.h"

LedStrip leds_L(11, 3);
LedStrip leds_M(33, 7);
LedStrip leds_R(32, 3);

void setup() {
  leds_L.begin();
  leds_M.begin();
  leds_R.begin();
  leds_L.setColor(LedSide::ALL, LedColor::WHITE);
  leds_M.setColor(LedSide::ALL, LedColor::WHITE);
  leds_R.setColor(LedSide::ALL, LedColor::WHITE);
  delay(1000);
  leds_M.blink(LedSide::ALL, LedColor::RED, 7, 200);
}

void loop() {
  leds_L.update();
  leds_M.update();
  leds_R.update();
}
