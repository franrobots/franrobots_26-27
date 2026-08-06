#include <Arduino.h>
#include "Motor.h"
#include "Robot.h"

motor mFR(Ain1_M1, Ain2_M1);
motor mRR(Ain1_M2, Ain2_M2);
motor mFL(Ain1_M3, Ain2_M3);
motor mRL(Ain1_M4, Ain2_M4);
Robot base(mFR, mRR, mFL, mRL);

void setup() {
  base.begin(12, 30000);
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
