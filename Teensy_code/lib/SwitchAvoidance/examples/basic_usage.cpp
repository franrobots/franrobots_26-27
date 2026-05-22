#include <Arduino.h>
#include "SwitchAvoidance.h"

SwitchAvoidance sw(13, 12);

void setup() {
  Serial.begin(115200);
  sw.begin(true);
}

void loop() {
  SwitchEvent ev = sw.poll();
  if (ev == SwitchEvent::LEFT) Serial.println("LEFT");
  else if (ev == SwitchEvent::RIGHT) Serial.println("RIGHT");
  else if (ev == SwitchEvent::BOTH) Serial.println("BOTH");
  delay(5);
}
