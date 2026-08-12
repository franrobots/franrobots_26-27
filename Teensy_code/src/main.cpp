#include <Arduino.h>
#include "VL53Mux12_FRAN.h"
#include "Scan360.h"
#include "Robot.h"
#include "RobotControl.h"
#include "BNO055_FranRobots.h"
#include "Encoder.h"
#include "ToFCalibration.h"
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
//
// Ciclo: CENTER -> ALIGN -> DECIDE -> TURN -> CENTER -> ALIGN -> DRIVE_TILE
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

MotionPhase lastPhase = MotionPhase::CENTER;

// Diagnostico: quantas vezes a baseline dos encoders foi rearmada desde o
// ultimo print. Se este numero for alto, o delta dFL/dRR nunca cresce -
// nao porque o encoder nao conta, mas porque o ponto de partida nao para
// quieto. Serve para separar "encoder mudo" de "baseline inquieta".
uint16_t phaseChangesSincePrint = 0;

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
// TESTE DE SENTIDO DOS MOTORES   (MOTOR_TEST_MODE em config.h)
//
// Aciona um motor por vez, ja com a inversao aplicada. Cada um tem que
// girar no sentido de FRENTE. Os que girarem ao contrario, inverta a flag
// MOTOR_INV_* correspondente. Rodar com as rodas no ar.
// ============================================================

void motorTestTick(uint32_t nowMs) {
  static uint8_t step = 0;
  static uint32_t nextMs = 0;

  if (nowMs < nextMs) return;
  nextMs = nowMs + MOTOR_TEST_STEP_MS;

  robotBase.stop(true);

  // Vai direto no motor: setLeftRight moveria os dois motores do lado.
  // As flags de inversao vivem no Robot, entao replico o sinal na mao.
  switch (step) {
    case 0:
      Serial.println("FL (M3) - deve girar para FRENTE");
      mFL.applyPWM(MOTOR_INV_FL ? -MOTOR_TEST_PWM : MOTOR_TEST_PWM);
      break;
    case 1:
      Serial.println("RL (M4) - deve girar para FRENTE");
      mRL.applyPWM(MOTOR_INV_RL ? -MOTOR_TEST_PWM : MOTOR_TEST_PWM);
      break;
    case 2:
      Serial.println("FR (M1) - deve girar para FRENTE");
      mFR.applyPWM(MOTOR_INV_FR ? -MOTOR_TEST_PWM : MOTOR_TEST_PWM);
      break;
    case 3:
      Serial.println("RR (M2) - deve girar para FRENTE");
      mRR.applyPWM(MOTOR_INV_RR ? -MOTOR_TEST_PWM : MOTOR_TEST_PWM);
      break;
    default:
      Serial.println("--- os quatro juntos: o robo deve andar para FRENTE ---");
      robotBase.move_tank(MOTOR_TEST_PWM, 0);
      break;
  }

  step = (step + 1) % 5;
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

  // ---------- Offsets de ToF gravados na EEPROM ----------
  // Sem isto todos os offsets ficam em zero e cada sensor entrega a sua
  // distancia crua, com o recuo de montagem embutido. Como snapshot() serve
  // EMA(raw - offset), esse vies entra direto em tudo que a navegacao usa -
  // e a centragem lateral e a mais sensivel: ela mede minLeft() - minRight()
  // e assume os dois lados simetricos. Se a esquerda estiver 8 mm mais
  // recuada que a direita, o robo "centraliza" 8 mm fora do centro, sempre.
  Serial.print("Offsets ToF (EEPROM)..... ");
  ToFCalibration::load(tof);
  Serial.println("carregados");

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
  // setInverted normaliza o sinal: FL e RR estao em lados espelhados do
  // chassi, e tanto traveledTicks() quanto o trim de equilibrio abaixo
  // assumem que os DOIS contam positivo andando para frente.
  Serial.print("Encoders (FL + RR)....... ");
  encFL.begin(true);
  encRR.begin(true);
  encFL.setInverted(ENC_FL_INVERTED);
  encRR.setInverted(ENC_RR_INVERTED);
  encFL.reset();
  encRR.reset();
  Serial.print("OK (FL ");
  Serial.print(ENC_FL_INVERTED ? "invertido" : "direto");
  Serial.print(", RR ");
  Serial.print(ENC_RR_INVERTED ? "invertido" : "direto");
  Serial.println(")");

  // ---------- Motores ----------
  // As inversoes tem que ser aplicadas: sem elas as duas rodas de um mesmo
  // lado podem girar em sentidos opostos e se anular. Ver MOTOR_INV_* em
  // config.h e o MOTOR_TEST_MODE para determina-las.
  Serial.print("Motores (4).............. ");
  robotBase.begin(PWM_RESOLUTION, PWM_FREQUENCY);
  robotBase.invertFL(MOTOR_INV_FL);
  robotBase.invertRL(MOTOR_INV_RL);
  robotBase.invertFR(MOTOR_INV_FR);
  robotBase.invertRR(MOTOR_INV_RR);
  robotBase.stop(true);
  Serial.print("OK (inv FL:");
  Serial.print(MOTOR_INV_FL ? "S" : "N");
  Serial.print(" RL:");
  Serial.print(MOTOR_INV_RL ? "S" : "N");
  Serial.print(" FR:");
  Serial.print(MOTOR_INV_FR ? "S" : "N");
  Serial.print(" RR:");
  Serial.print(MOTOR_INV_RR ? "S" : "N");
  Serial.println(")");

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
  controller.setAlignMs(PHASE_ALIGN_MAX_MS);
  controller.setTurn90Ms(PHASE_TURN_MAX_MS);
  controller.setTileDriveMs(PHASE_DRIVE_MAX_MS);
  controller.setFrontStopMm(FRONT_STOP_MM);
  CenteringConfig centering;
  centering.frontTargetMm = ALIGN_FRONT_TARGET_MM;
  centering.backTargetMm  = ALIGN_BACK_TARGET_MM;
  centering.refValidMm    = CENTER_REF_VALID_MM;
  centering.tolMm         = CENTER_TOL_MM;
  centering.frontMinMm    = FRONT_STOP_MM;
  centering.backMinMm     = ALIGN_BACK_MIN_MM;
  centering.kp            = CENTER_POS_KP;
  centering.minPwm        = CENTER_POS_MIN_PWM;
  centering.maxPwm        = CENTER_POS_MAX_PWM;
  centering.maxMs         = PHASE_CENTER_MAX_MS;
  controller.setCentering(centering);
  Serial.println("OK");

  lastPhase = MotionPhase::CENTER;
  lastControlMs = millis();

  if (MOTOR_TEST_MODE) {
    Serial.println();
    Serial.println("--------------------------------------------------------");
    Serial.println("  TESTE DE SENTIDO DOS MOTORES - rodas no ar!");
    Serial.println("  Um motor por vez. Cada um deve girar para FRENTE.");
    Serial.println("  Os que girarem ao contrario: inverta MOTOR_INV_* em");
    Serial.println("  config.h. No fim os quatro giram juntos.");
    Serial.println("--------------------------------------------------------");
    Serial.println();
    return;
  }

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

  // ---------- Teste de sentido dos motores ----------
  if (MOTOR_TEST_MODE) {
    motorTestTick(nowMs);
    return;
  }

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
  in.tofBackMm  = scan.minBack();   // referencia de re para o CENTER

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
    phaseChangesSincePrint++;
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
  //
  // PRE-REQUISITO: os dois encoders contam positivo para frente
  // (Encoder::setInverted no setup). Sem isso (dFl - dRr) vira a SOMA das
  // distancias em vez da diferenca, cresce sem parar e satura o termo em
  // ENC_OUT_MAX ja no primeiro terco do ladrilho.
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

      // Teto SEPARADO para a correcao no avanco. move_tank faz
      // left = DRIVE_PWM + turn e right = DRIVE_PWM - turn: se turn chegar
      // perto de DRIVE_PWM, a roda de um lado zera e a do outro satura -
      // o robo para de andar e passa a girar no meio do ladrilho. Este
      // limite mantem a correcao como direcao, nao como giro.
      if (turnRaw >  DRIVE_TURN_MAX) turnRaw =  DRIVE_TURN_MAX;
      if (turnRaw < -DRIVE_TURN_MAX) turnRaw = -DRIVE_TURN_MAX;
    } else if (out.phase == MotionPhase::CENTER) {
      // Posicionamento longitudinal. O PWM vem do RobotControl, que e quem
      // tem o erro em mm; aqui so se segura o rumo com o yaw.
      //
      // Sem centragem lateral e sem trim de encoder: os dois foram pensados
      // para o avanco de um ladrilho. O termo de centragem em particular
      // inverte de efeito andando de re - corrigiria para o lado errado.
      linearCmd = out.linearPwm;
      turnRaw = yawTerm;
      if (turnRaw >  DRIVE_TURN_MAX) turnRaw =  DRIVE_TURN_MAX;
      if (turnRaw < -DRIVE_TURN_MAX) turnRaw = -DRIVE_TURN_MAX;
    } else {
      // ALIGN / TURN: gira parado ate bater no rumo alvo.
      //
      // A centragem lateral fica de fora de proposito: com traccao
      // diferencial o robo nao anda de lado, entao parado nao ha o que
      // corrigir. Ela so faz sentido durante o DRIVE_TILE.
      linearCmd = 0;
      turnRaw = yawTerm;

      // Parado, as quatro rodas tem que arrastar de lado. Abaixo de
      // TURN_MIN_PWM o motor nao vence o atrito estatico e o robo nao sai
      // do lugar; acima dele, uma vez solto, gira dezenas de graus de uma
      // vez. Os dois regimes precisam de tratamento diferente.
      //
      // So nesta fase: no DRIVE_TILE e no CENTER as rodas ja estao
      // girando - o atrito estatico ja foi vencido - e forcar um piso
      // desses jogaria o robo para fora da linha.
      const float absErr = fabsf(yawErr);
      const float dir = (yawErr > 0.0f) ? 1.0f : -1.0f;

      if (absErr <= YAW_TOL_DEG) {
        // Chegou. Solta, em vez de ficar chutando o alvo.
        turnRaw = 0.0f;
      } else if (absErr > TURN_FINE_DEG) {
        // Erro grande (o giro de 90 graus): acionamento continuo, com piso
        // para garantir o arranque.
        if (fabsf(turnRaw) < TURN_MIN_PWM) turnRaw = dir * TURN_MIN_PWM;
      } else {
        // Erro pequeno: pulsos. O pulso da o empurrao que vence o atrito e
        // acaba antes de o robo girar demais - com acionamento continuo
        // nao ha meio termo entre nao mover e passar longe.
        const uint32_t period = TURN_PULSE_ON_MS + TURN_PULSE_OFF_MS;
        const bool pulseOn = (nowMs % period) < TURN_PULSE_ON_MS;
        turnRaw = pulseOn ? (dir * TURN_MIN_PWM) : 0.0f;
      }
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

    // Malha de rumo
    Serial.print("  Yaw:");
    Serial.print(in.yawDeg, 1);
    Serial.print(" alvo:");
    Serial.print(targetYaw, 0);
    Serial.print(" erro:");
    Serial.print(dbgYawErr, 1);
    Serial.println();

    // Odometria. "abs" e a contagem ACUMULADA desde o boot - o mesmo valor
    // que o basic_usage.cpp imprime. "d" e a diferenca contra a baseline,
    // que e rearmada a cada troca de fase; "trocas" conta quantas vezes
    // isso aconteceu desde o print anterior.
    //
    // COMO LER: abs parado = ISR nao dispara (problema eletrico/pinagem).
    // abs crescendo com d em zero e trocas alto = a baseline nao para
    // quieta, o encoder esta bom e o problema e a maquina de fases.
    Serial.print("  Enc abs FL:");
    Serial.print(flTicks);
    Serial.print(" RR:");
    Serial.print(rrTicks);
    Serial.print(" | d FL:");
    Serial.print(dFl);
    Serial.print(" RR:");
    Serial.print(dRr);
    Serial.print(" med:");
    Serial.print((dFl + dRr) / 2);
    Serial.print("/");
    Serial.print(ENCODER_TICKS_PER_TILE);
    Serial.print(" | trocas:");
    Serial.print(phaseChangesSincePrint);
    Serial.println();
    phaseChangesSincePrint = 0;

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
