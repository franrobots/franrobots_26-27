#include <Arduino.h>
#include "Motor.h"
  

motor m(Ain1_M1, Ain2_M1, PWM_M1);

void setup() {
  m.begin(12, 20000);
  m.setDeadband(800);
}

void loop() {
  m.applyPWM(3000);
  delay(600);
  m.applyPWM(-3000);
  delay(600);
  m.stopMotor(true);
  delay(600);
}
