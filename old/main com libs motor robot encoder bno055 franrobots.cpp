#include <Arduino.h>
#include "config.h"

#include "motor.h"
#include "Robot.h"

#include "Encoder.h"
#include "BNO055_FranRobots.h"

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
// Sensores (libs separadas)
// =======================
// Encoders: FL e RR (como você disse)
Encoder encFL(ENC1);
Encoder encRR(ENC2);

// BNO055 endereço 0x29
BNO055_FranRobots imu(0x29);

// =======================
// Demo 12 bits
// =======================
static constexpr int16_t SPEED_FWD  = 2500; // 0..4095
static constexpr int16_t SPEED_TURN = 2200;

static void printSensors() {
  Serial.print("Yaw: ");
  Serial.print(imu.getYaw(), 1);
  Serial.print(" | FL ticks: ");
  Serial.print(encFL.read());
  Serial.print(" | RR ticks: ");
  Serial.println(encRR.read());
}

static void demoMoves() {
  Serial.println("\n--- DEMO ---");

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
    Serial.println("Boot - Robot + Encoder + BNO055_FranRobots");
  }

  // =======================
  // Motores / Robot
  // =======================
  robot.begin(PWM_RESOLUTION, PWM_FREQUENCY); // 12 bits / 20kHz

  // Deadband típico em 12 bits (ajuste no robô)
  mFL.setDeadband(300);
  mRL.setDeadband(300);
  mFR.setDeadband(300);
  mRR.setDeadband(300);

  // Ganho por motor (se precisar equalizar)
  mFL.setMotorGain(1.00f);
  mRL.setMotorGain(1.00f);
  mFR.setMotorGain(1.00f);
  mRR.setMotorGain(1.00f);

  // Inversão comum do lado direito
  robot.invertFR(true);
  robot.invertRR(true);

  // =======================
  // Encoders
  // =======================
  encFL.begin(true);
  encRR.begin(true);

  // Zera contagem pra começar limpo
  encFL.reset();
  encRR.reset();

  // =======================
  // BNO055
  // =======================
  if (!imu.begin(Wire, 400000)) {
    Serial.println("ERRO: BNO055 nao encontrado no endereco 0x29");
    while (1) delay(10);
  }
  imu.zeroYaw();

  Serial.println("Ready.");
}

void loop() {
  // Monitoramento rápido
  printSensors();
  delay(150);

  // Movimento de demonstração
  demoMoves();
}