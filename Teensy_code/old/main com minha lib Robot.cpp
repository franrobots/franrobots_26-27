#include <Arduino.h>
#include "config.h"
#include "motor.h"
#include "Robot.h"

// =======================
// Motores (direção compartilhada por pares)
// =======================

// LADO ESQUERDO (Motores 1 e 2) -> IN00A/IN00B
motor mFL(IN00A, IN00B, PWM_M1);
motor mRL(IN00A, IN00B, PWM_M2);

// LADO DIREITO (Motores 3 e 4) -> IN01A/IN01B
motor mFR(IN01A, IN01B, PWM_M3);
motor mRR(IN01A, IN01B, PWM_M4);

// Robot (ordem: FL, RL, FR, RR)
Robot robot(mFL, mRL, mFR, mRR);

// =======================
// Demo
// =======================
static constexpr int16_t SPEED_FWD  = 2500; // 12 bits: 0..4095
static constexpr int16_t SPEED_TURN = 2200;

static void demoMoves() {
  Serial.println("Forward");
  robot.drive(SPEED_FWD, 0);
  delay(800);

  Serial.println("Stop (coast)");
  robot.stop(false);
  delay(400);

  Serial.println("Backward");
  robot.drive(-SPEED_FWD, 0);
  delay(800);

  Serial.println("Stop (brake)");
  robot.stop(true);
  delay(500);

  Serial.println("TurnLeft");
  robot.turnLeft(SPEED_TURN);
  delay(700);

  Serial.println("Stop");
  robot.stop(false);
  delay(400);

  Serial.println("TurnRight");
  robot.turnRight(SPEED_TURN);
  delay(700);

  Serial.println("Stop");
  robot.stop(false);
  delay(1000);
}

void setup() {
  if (DEBUG_SERIAL) {
    Serial.begin(115200);
    delay(200);
    Serial.println("Booting Robot (12-bit PWM)...");
  }

  // ✅ Usa as configs do seu arquivo
  robot.begin(PWM_RESOLUTION, PWM_FREQUENCY);

  // Deadband típico em 12 bits (ajuste fino no robô)
  mFL.setDeadband(300);
  mRL.setDeadband(300);
  mFR.setDeadband(300);
  mRR.setDeadband(300);

  // Ganho por motor (caso precise equalizar)
  mFL.setMotorGain(1.00f);
  mRL.setMotorGain(1.00f);
  mFR.setMotorGain(1.00f);
  mRR.setMotorGain(1.00f);

  // Inversão (normalmente o lado direito precisa inverter)
  robot.invertFR(true);
  robot.invertRR(true);

  if (DEBUG_SERIAL) Serial.println("Robot ready.");
}

void loop() {
  demoMoves();
}