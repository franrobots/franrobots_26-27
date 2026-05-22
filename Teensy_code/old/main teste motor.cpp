#include <Arduino.h>
#include "MotorPairDirTB6612_Teensy.h"
#include "Drive4_PairedDir_Teensy.h"

// Direção compartilhada por pares
constexpr uint8_t IN00A = 6;
constexpr uint8_t IN00B = 7;
constexpr uint8_t IN01A = 8;
constexpr uint8_t IN01B = 9;

// PWM individual
constexpr uint8_t PWM_M1 = 2;
constexpr uint8_t PWM_M2 = 3;
constexpr uint8_t PWM_M3 = 4;
constexpr uint8_t PWM_M4 = 5;

int motorValueCorrection(int v) {
  return v;
}

// Par 1: Motores 1 e 2
MotorPairDirTB6612 pair12(IN00A, IN00B, PWM_M1, PWM_M2);
// Par 2: Motores 3 e 4
MotorPairDirTB6612 pair34(IN01A, IN01B, PWM_M3, PWM_M4);

Drive4_PairedDir drive(pair12, pair34);

void setup() {
  drive.begin(12, 20000);
  drive.setCorrectionFn(motorValueCorrection);

  drive.setDeadbandAll(80);
  drive.setStopModeAll(MotorPairDirTB6612::StopMode::Brake);

  // Ganhos por motor (trim)
  // Exemplo: M2 é um pouco mais forte -> reduz; M3 é mais fraco -> aumenta
  drive.setMotorGains(
    1.00f, // M1
    0.95f, // M2
    1.07f, // M3
    1.00f  // M4
  );
}

void loop() {
  drive.moveTank(1500, 1500, true);   // reto
  delay(700);

  drive.moveTank(1500, -1500, true);  // gira
  delay(700);

  drive.stopAll(MotorPairDirTB6612::StopMode::Brake);
  delay(500);
}
