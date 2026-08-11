#include <Arduino.h>
#include "VL53Mux12_FRAN.h"
#include "Scan360.h"
#include "Robot.h"
#include "RobotControl.h"
#include "BNO055_FranRobots.h"
#include "Encoder.h"
#include "config.h"

// ============================================================
// SPRINT 3: Navegacao com ToF + BNO055 + Encoders + PID
//
// Divisao de responsabilidades:
//   RobotControl -> decide O QUE fazer (fase, rumo alvo, fim do ladrilho)
//   main.cpp     -> decide COMO fazer (PID que gera o PWM final)
//
// Malhas fechadas ativas:
//   yaw     (BNO055)   -> rumo. Referencia principal de direcao.
//   encoder (2 rodas)  -> distancia do ladrilho + trim de equilibrio.
//   ToF     (11 sens.) -> paredes livres + centragem no corredor.
// ============================================================

// ========== SENSORES ==========
VL53Mux12_FRAN tof(TCA_A, TCA_B);
Scan360 scan;
ScanToF12 raw;

// Nao chamar de "imu": colide com o namespace imu:: do imumaths.h,
// arrastado pelo Adafruit_BNO055.
BNO055_FranRobots bno(BNO055_ADDRESS);
Encoder encFL(ENC1_A, ENC1_B);
Encoder encRR(ENC2_A, ENC2_B);

// ========== MOTORES ==========
motor mFL(Ain1_M3, Ain2_M3);
motor mRL(Ain1_M4, Ain2_M4);
motor mFR(Ain1_M1, Ain2_M1);
motor mRR(Ain1_M2, Ain2_M2);
Robot robotBase(mFL, mRL, mFR, mRR);

// ========== CONTROLADOR ==========
RobotControl controller;

// ============================================================
// PID
// ============================================================

struct Pid {
  float kp = 0.0f, ki = 0.0f, kd = 0.0f;
  float iMax = 0.0f, outMax = 0.0f;

  float integ = 0.0f;
  float prevErr = 0.0f;
  bool hasPrev = false;
  uint32_t lastMs = 0;

  void configure(float p, float i, float d, float imax, float omax) {
    kp = p; ki = i; kd = d; iMax = imax; outMax = omax;
    reset();
  }

  // Chamar sempre que o alvo mudar (troca de fase): zera o historico para
  // o integral de uma fase nao vazar para a proxima.
  void reset() {
    integ = 0.0f;
    prevErr = 0.0f;
    hasPrev = false;
    lastMs = 0;
  }

  float update(float err, uint32_t nowMs) {
    // dt medido, com limites: protege contra divisao por zero e contra
    // um salto gigante no derivativo depois de uma pausa longa.
    float dt = (lastMs == 0) ? (CONTROL_PERIOD_MS / 1000.0f)
                             : (nowMs - lastMs) / 1000.0f;
    if (dt < 0.001f) dt = 0.001f;
    if (dt > 0.200f) dt = 0.200f;
    lastMs = nowMs;

    integ += err * dt;
    if (integ >  iMax) integ =  iMax;
    if (integ < -iMax) integ = -iMax;

    float deriv = 0.0f;
    if (hasPrev) deriv = (err - prevErr) / dt;
    prevErr = err;
    hasPrev = true;

    float o = (kp * err) + (ki * integ) + (kd * deriv);
    if (o >  outMax) o =  outMax;
    if (o < -outMax) o = -outMax;
    return o;
  }
};

Pid yawPid;   // erro em graus  -> comando de giro
Pid encPid;   // erro em ticks  -> trim de giro

// ============================================================
// ESTADO
// ============================================================

bool imuOk = false;
float yawNow = 0.0f;

int32_t flTicks = 0;
int32_t rrTicks = 0;
int32_t tileStartFl = 0;   // baseline dos encoders no inicio do ladrilho
int32_t tileStartRr = 0;

MotionPhase lastPhase = MotionPhase::DECIDE;

// Ultimos comandos aplicados (para o debug refletir o que foi para o motor)
int16_t appliedLinear = 0;
int16_t appliedTurn = 0;
float dbgYawErr = 0.0f;
float dbgYawTerm = 0.0f;
float dbgEncTerm = 0.0f;
float dbgCenterTerm = 0.0f;

uint32_t lastTofUpdateMs = 0;
uint32_t lastImuMs = 0;
uint32_t lastControlMs = 0;
uint32_t lastPrintMs = 0;
uint32_t lastWarnMs = 0;

// ============================================================
// AUXILIARES
// ============================================================

float headingToYaw(Heading h) {
  if (h == Heading::NORTH) return 0.0f;
  if (h == Heading::EAST)  return 90.0f;
  if (h == Heading::SOUTH) return 180.0f;
  return 270.0f;
}

const char* headingToString(Heading h) {
  if (h == Heading::NORTH) return "N";
  if (h == Heading::EAST)  return "E";
  if (h == Heading::SOUTH) return "S";
  return "W";
}

// Menor caminho angular: resultado sempre em (-180, +180].
// Positivo = precisa AUMENTAR o yaw (girar no sentido horario).
float shortestAngle(float deg) {
  while (deg >  180.0f) deg -= 360.0f;
  while (deg < -180.0f) deg += 360.0f;
  return deg;
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("========================================================");
  Serial.println("  SPRINT 3: ToF + BNO055 + Encoders + PID");
  Serial.println("========================================================");
  Serial.println();

  // ---------- ToF ----------
  Serial.print("ToF (11 sensores)........ ");
  if (!tof.begin(Wire, I2C_FREQUENCY, 40)) {
    Serial.println("ERRO");
    while (1) {
      Serial.println("HALTED: ToF nao respondeu");
      delay(1000);
    }
  }
  tof.setEmaAlpha(0.25f);
  Serial.println("OK");

  // ---------- BNO055 ----------
  // Compartilha o mesmo barramento I2C dos multiplexadores ToF.
  Serial.print("BNO055 (IMU)............. ");
  imuOk = bno.begin(Wire, I2C_FREQUENCY);
  if (imuOk) {
    delay(50);
    // Zera o rumo: a orientacao atual passa a ser NORTH (0 graus), que e
    // o que RobotControl assume como pose inicial.
    bno.zeroYaw();
    yawNow = bno.getYaw360();
    Serial.println("OK (yaw zerado = NORTH)");
  } else {
    Serial.println("FALHOU - modo degradado (giro por tempo)");
  }

  // ---------- Encoders ----------
  // A ordem importa: o primeiro begin() pega a ISR 0, o segundo a ISR 1.
  Serial.print("Encoders (FL + RR)....... ");
  encFL.begin(true);
  encRR.begin(true);
  encFL.reset();
  encRR.reset();
  Serial.println("OK");

  // ---------- Motores ----------
  Serial.print("Motores (4).............. ");
  robotBase.begin(PWM_RESOLUTION, PWM_FREQUENCY);
  robotBase.stop(true);
  Serial.println("OK");

  // ---------- PID ----------
  Serial.print("PID (yaw + encoder)...... ");
  yawPid.configure(YAW_KP, YAW_KI, YAW_KD, YAW_I_MAX, YAW_OUT_MAX);
  encPid.configure(ENC_KP, ENC_KI, ENC_KD, ENC_I_MAX, ENC_OUT_MAX);
  Serial.println("OK");

  // ---------- Navegacao ----------
  Serial.print("RobotControl............. ");
  controller.begin(0, 0, Heading::NORTH);
  controller.setTicksPerTile(ENCODER_TICKS_PER_TILE);
  // Com IMU e encoders reais, estes tempos viram apenas watchdog.
  controller.setPreAlignMs(PHASE_PREALIGN_MAX_MS);
  controller.setTurn90Ms(PHASE_TURN_MAX_MS);
  controller.setTileDriveMs(PHASE_DRIVE_MAX_MS);
  controller.setFrontStopMm(FRONT_STOP_MM);
  Serial.println("OK");

  lastPhase = MotionPhase::DECIDE;
  lastControlMs = millis();

  Serial.println();
  Serial.println("--------------------------------------------------------");
  Serial.print("Malhas: yaw=");
  Serial.print(imuOk ? "BNO055" : "TEMPO(degradado)");
  Serial.print("  distancia=encoder(");
  Serial.print(ENCODER_TICKS_PER_TILE);
  Serial.print(" ticks/ladrilho)  centragem=ToF");
  Serial.println();
  Serial.println("Sistema pronto.");
  Serial.println("--------------------------------------------------------");
  Serial.println();
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  const uint32_t nowMs = millis();

  // ---------- ToF: round-robin, um sensor por iteracao ----------
  tof.update();

  if (nowMs - lastTofUpdateMs >= 50) {
    lastTofUpdateMs = nowMs;
    tof.snapshot(raw);
    scan.s = raw;
  }

  // ---------- IMU: cadencia propria, leitura I2C nao e barata ----------
  if (imuOk && (nowMs - lastImuMs >= IMU_PERIOD_MS)) {
    lastImuMs = nowMs;
    yawNow = bno.getYaw360();
  }

  // ---------- Aviso de lado cego (1x/s) ----------
  if (nowMs - lastWarnMs >= 1000) {
    if (scan.validFront() == 0 || scan.validLeft() == 0 ||
        scan.validRight() == 0 || scan.validBack() == 0) {
      lastWarnMs = nowMs;
      Serial.print("AVISO: lado sem leitura ToF -> F:");
      Serial.print(scan.validFront());
      Serial.print("/3 L:");
      Serial.print(scan.validLeft());
      Serial.print("/3 R:");
      Serial.print(scan.validRight());
      Serial.print("/3 B:");
      Serial.print(scan.validBack());
      Serial.println("/2");
    }
  }

  // ==========================================================
  // PASSO DE CONTROLE - periodo fixo, para o PID ter dt estavel
  // ==========================================================
  if (nowMs - lastControlMs < CONTROL_PERIOD_MS) return;
  lastControlMs = nowMs;

  // ---------- Leitura dos encoders ----------
  flTicks = encFL.read();
  rrTicks = encRR.read();

  // ---------- Montagem do PlannerInput ----------
  PlannerInput in;

  // Paredes livres: menor valor valido de cada grupo de ToF.
  in.frontFree = scan.minFront() > TOF_WALL_CLEAR_MM;
  in.leftFree  = scan.minLeft()  > TOF_WALL_CLEAR_MM;
  in.rightFree = scan.minRight() > TOF_WALL_CLEAR_MM;
  in.backFree  = scan.minBack()  > TOF_WALL_CLEAR_MM;

  const uint16_t leftMm  = scan.minLeft();
  const uint16_t rightMm = scan.minRight();
  const bool bothWalls = (leftMm  > 0 && leftMm  <= CENTER_VALID_MM) &&
                         (rightMm > 0 && rightMm <= CENTER_VALID_MM);
  in.tofLeftMm  = bothWalls ? leftMm  : 0;
  in.tofRightMm = bothWalls ? rightMm : 0;
  in.tofFrontMm = scan.minFront();

  // Agora sao dados REAIS: o RobotControl passa a fechar as malhas dele
  // (transicao de fase por erro de yaw, fim do ladrilho por ticks).
  in.yawDeg     = imuOk ? yawNow : headingToYaw(controller.pose().heading);
  in.encFlTicks = flTicks;
  in.encRrTicks = rrTicks;

  in.sensedTile = TileType::UNKNOWN;   // sem sensor de cor nesta sprint
  in.victimLeft = 0;
  in.victimRight = 0;
  in.victimConfLeft = 0;
  in.victimConfRight = 0;
  in.nowMs = nowMs;

  // ---------- Planejamento ----------
  PlannerOutput out = controller.update(in);

  // ---------- Troca de fase: rearma os PIDs ----------
  if (out.phase != lastPhase) {
    lastPhase = out.phase;
    yawPid.reset();
    encPid.reset();
    // Baseline dos encoders para o trim de equilibrio do ladrilho.
    tileStartFl = flTicks;
    tileStartRr = rrTicks;
  }

  // ---------- Erro de rumo ----------
  // Alvo vem do RobotControl; durante o giro difere de pose().heading.
  const float targetYaw = headingToYaw(controller.targetHeading());
  const float yawErr = shortestAngle(targetYaw - in.yawDeg);
  dbgYawErr = yawErr;

  // Termo de guinada. Convencao: positivo = girar no sentido horario
  // (sentido em que o yaw do BNO aumenta).
  float yawTerm = 0.0f;
  if (imuOk) {
    yawTerm = yawPid.update(yawErr, nowMs);
  } else {
    // Sem IMU o yaw sintetico nao converge, mas o SINAL do erro ainda
    // aponta o lado certo. Gira com PWM fixo ate o watchdog da fase.
    if (fabsf(yawErr) > YAW_TOL_DEG) {
      yawTerm = (yawErr > 0.0f) ? YAW_OUT_MAX * 0.8f : -YAW_OUT_MAX * 0.8f;
    }
  }
  dbgYawTerm = yawTerm;

  // ---------- Termo de equilibrio das rodas ----------
  // Se a roda esquerda andou mais que a direita, o robo abriu para a
  // direita: corrige girando para a esquerda (termo negativo).
  const int32_t dFl = flTicks - tileStartFl;
  const int32_t dRr = rrTicks - tileStartRr;
  float encTerm = ENC_BALANCE_SIGN * encPid.update((float)(dFl - dRr), nowMs);
  dbgEncTerm = encTerm;

  // ---------- Termo de centragem entre paredes ----------
  // leftMm > rightMm significa colado na parede direita: corrige para a
  // esquerda (termo negativo).
  float centerTerm = 0.0f;
  if (bothWalls) {
    centerTerm = -CENTER_KP * (float)((int32_t)leftMm - (int32_t)rightMm);
    if (centerTerm >  CENTER_OUT_MAX) centerTerm =  CENTER_OUT_MAX;
    if (centerTerm < -CENTER_OUT_MAX) centerTerm = -CENTER_OUT_MAX;
  }
  dbgCenterTerm = centerTerm;

  // ---------- Composicao do comando ----------
  int16_t linearCmd = 0;
  float turnRaw = 0.0f;

  if (out.command == MotionCommand::RETREAT_ONE) {
    linearCmd = out.linearPwm;   // negativo, definido pelo RobotControl
    turnRaw = 0.0f;
  } else if (out.command == MotionCommand::DRIVE_CLOSED_LOOP) {
    if (out.phase == MotionPhase::DRIVE_TILE) {
      // Reta: avanca e soma as tres correcoes.
      linearCmd = DRIVE_PWM;
      turnRaw = yawTerm + encTerm + centerTerm;
    } else {
      // PRE_ALIGN / TURN_90: gira parado ate bater no rumo alvo.
      linearCmd = 0;
      turnRaw = yawTerm;
    }
  }

  // TURN_SIGN converte "rotacao desejada" na convencao de move_tank.
  float turnScaled = TURN_SIGN * turnRaw;
  if (turnScaled >  YAW_OUT_MAX) turnScaled =  YAW_OUT_MAX;
  if (turnScaled < -YAW_OUT_MAX) turnScaled = -YAW_OUT_MAX;
  const int16_t turnCmd = (int16_t)turnScaled;

  // ---------- Atuacao ----------
  if (out.command == MotionCommand::HOLD) {
    robotBase.stop(true);
    appliedLinear = 0;
    appliedTurn = 0;
  } else {
    robotBase.move_tank(linearCmd, turnCmd);
    appliedLinear = linearCmd;
    appliedTurn = turnCmd;
  }

  // ==========================================================
  // DEBUG
  // ==========================================================
  if (nowMs - lastPrintMs >= 500) {
    lastPrintMs = nowMs;

    // Navegacao
    Serial.print("Fase:");
    Serial.print(RobotControl::toString(out.phase));
    Serial.print(" Pose:(");
    Serial.print(out.pose.x);
    Serial.print(",");
    Serial.print(out.pose.y);
    Serial.print(",");
    Serial.print(headingToString(out.pose.heading));
    Serial.print(") Alvo:");
    Serial.print(headingToString(controller.targetHeading()));
    Serial.print(" Nav:");
    Serial.print(RobotControl::toString(out.source));
    Serial.print(" PWM(");
    Serial.print(appliedLinear);
    Serial.print(",");
    Serial.print(appliedTurn);
    Serial.println(")");

    // Malha de rumo + odometria
    Serial.print("  Yaw:");
    Serial.print(in.yawDeg, 1);
    Serial.print(" alvo:");
    Serial.print(targetYaw, 0);
    Serial.print(" erro:");
    Serial.print(dbgYawErr, 1);
    Serial.print(" | Enc dFL:");
    Serial.print(dFl);
    Serial.print(" dRR:");
    Serial.print(dRr);
    Serial.print("/");
    Serial.print(ENCODER_TICKS_PER_TILE);
    Serial.println();

    // Termos do PID
    Serial.print("  PID yaw:");
    Serial.print(dbgYawTerm, 0);
    Serial.print(" enc:");
    Serial.print(dbgEncTerm, 0);
    Serial.print(" centro:");
    Serial.print(dbgCenterTerm, 0);
    Serial.println();

    // ToF
    Serial.print("  ToF F:");
    Serial.print(scan.minFront());
    Serial.print(" L:");
    Serial.print(scan.minLeft());
    Serial.print(" R:");
    Serial.print(scan.minRight());
    Serial.print(" B:");
    Serial.print(scan.minBack());
    Serial.print(" | Livre F:");
    Serial.print(in.frontFree ? "S" : "N");
    Serial.print(" L:");
    Serial.print(in.leftFree ? "S" : "N");
    Serial.print(" R:");
    Serial.print(in.rightFree ? "S" : "N");
    Serial.print(" B:");
    Serial.print(in.backFree ? "S" : "N");
    Serial.print(" | Ok ");
    Serial.print(scan.validFront());
    Serial.print(scan.validLeft());
    Serial.print(scan.validRight());
    Serial.print(scan.validBack());
    Serial.println();
  }
}
