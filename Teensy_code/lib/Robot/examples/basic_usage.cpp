#include <Arduino.h>
#include "Motor.h"
#include "Robot.h"

motor mFL(29, 43, 2);
motor mRL(6, 7, 3);
motor mFR(28, 24, 4);
motor mRR(9, 8, 5);
Robot base(mFL, mRL, mFR, mRR);

void setup() {
  base.begin(12, 20000);
  mFL.setDeadband(800);
  mRL.setDeadband(800);
  mFR.setDeadband(800);
  mRR.setDeadband(800);
}

void loop() {
  base.move_tank(3000, 0);
  delay(600);
  base.move_tank(0, 3000);
  delay(500);
  base.stop(true);
  delay(600);
}
