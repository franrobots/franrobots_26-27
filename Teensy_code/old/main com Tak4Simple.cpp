#include <Arduino.h>
#include "Tank4Simple.h"

constexpr uint8_t IN00A = 6;
constexpr uint8_t IN00B = 7;
constexpr uint8_t IN01A = 8;
constexpr uint8_t IN01B = 9;

constexpr uint8_t PWM_M1 = 2;
constexpr uint8_t PWM_M2 = 3;
constexpr uint8_t PWM_M3 = 4;
constexpr uint8_t PWM_M4 = 5;

int motorValueCorrection(int v) { return v; }

Tank4Simple drive(IN00A, IN00B, IN01A, IN01B, PWM_M1, PWM_M2, PWM_M3, PWM_M4);

void setup() {
  drive.begin(12, 20000);

  drive.setCorrectionFn(motorValueCorrection);

  drive.setMotorGains(1.00f, 0.96f, 1.05f, 1.00f);

  // ✅ ajuste típico (você calibra na prática)
  drive.setDeadband(120);
}

void loop() {
  drive.moveTank(80, 80, true);     // mesmo pequeno, vai partir por causa do deadband
  delay(700);

  drive.stop(true);
  delay(400);
}
