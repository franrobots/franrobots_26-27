#include <Arduino.h>
#include "Motor.h"

motor m(6, 7, 2);

void setup() {
  m.begin(12, 20000);
  m.setDeadband(120);
}

void loop() {
  m.applyPWM(600);
  delay(600);
  m.applyPWM(-600);
  delay(600);
  m.stopMotor(true);
  delay(600);
}
