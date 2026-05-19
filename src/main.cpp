#include <Arduino.h>
#include "Motor.h"

motor m(15, 6, 13);
motor m2(14, 5, 22);

void setup() {
  m.begin(12, 20000);
  m.setDeadband(120);
  m2.begin(12, 20000);
  m2.setDeadband(120);
}

void loop() {
  m.applyPWM(600);
  m2.applyPWM(600);
  delay(600);
  m.applyPWM(-600);
  m2.applyPWM(-600);
  delay(600);
  m.stopMotor(true);
  m2.stopMotor(true);
  delay(600);
}
