#include <Arduino.h>
#include "Motor.h"
#include "Robot.h"

motor mFR(Ain1_M1, Ain2_M1, PWM_M3);
motor mRR(Ain1_M2, Ain2_M2, PWM_M2);
motor mFL(Ain1_M3, Ain2_M3, PWM_M1);
motor mRL(Ain1_M4, Ain2_M4, PWM_M2);
Robot base(mFR, mRR, mFL, mRL);

void setup() {
  base.begin(12, 20000);
  mFL.setDeadband(100);
  mRL.setDeadband(100);
  mFR.setDeadband(100);
  mRR.setDeadband(100);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    base.move_tank(200, 0);
    delay(3000);
    base.stop(true);
    delay(3000);
    }else{
        base.stop(true);
    }
}
