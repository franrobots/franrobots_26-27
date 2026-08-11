#include <Arduino.h>
#include "VL53Mux12_FRAN.h"
#include "Scan360.h"
#include "ToFCalibration.h"
#include "ColorCalibration.h"
#include "ReflectancePlate.h"
#include "RobotControl.h"
#include "BNO055_FranRobots.h"
#include "Encoder.h"
#include "OpenMVCamera.h"
#include "Robot.h"
#include "SwitchAvoidance.h"
#include "LedStrip.h"
#include "ServoKit.h"
#include "config.h"

VL53Mux12_FRAN tof(TCA_A, TCA_B);
Scan360 scan;
ScanToF12 raw;
 
static const uint8_t kFloorPins[] = {C9_PIN, LDR_RED,LDR_GREEN, LDR_BLUE};
ReflectancePlate floorSensor(kFloorPins, 4);

BNO055_FranRobots imub(BNO055_ADDRESS);
Encoder encFL(ENC1_A, ENC1_B);
Encoder encRR(ENC2_A, ENC2_B);

OpenMVCamera camL(OPENMVL);
OpenMVCamera camR(OPENMVR);

motor mFL(Ain1_M3, Ain2_M3);
motor mRL(Ain1_M4, Ain2_M4);
motor mFR(Ain1_M1, Ain2_M1);
motor mRR(Ain1_M2, Ain2_M2);
Robot robotBase(mFL, mRL, mFR, mRR);
LedStrip led_LEFT(LED_LEFT, 3);
LedStrip led_MIDDLE(LED_MIDDLE, 7);
LedStrip led_RIGHT(LED_RIGHT, 3);
ServoKit servoKit;
SwitchAvoidance bumper(SW_LEFT, SW_RIGHT);

RobotControl controller;
uint32_t nextVictimActionAtMs = 0;
bool pauseByButton = false;
bool lastButtonRead = HIGH;
uint32_t lastButtonEdgeMs = 0;
bool pauseLedOn = false;
uint32_t pauseLedToggleMs = 0;

static bool hasWallOnLeftVictimSide(const Scan360& s) {
  const uint8_t idx = (uint8_t)ToFId::LC;
  return s.s.ok[idx] && s.s.mm[idx] > 0 && s.s.mm[idx] <= VICTIM_WALL_CONFIRM_MM;
}

static bool hasWallOnRightVictimSide(const Scan360& s) {
  const uint8_t idx = (uint8_t)ToFId::RC;
  return s.s.ok[idx] && s.s.mm[idx] > 0 && s.s.mm[idx] <= VICTIM_WALL_CONFIRM_MM;
}

static void handleSwitchObstacle(SwitchEvent ev) {
  if (ev == SwitchEvent::NONE) return;

  robotBase.stop(true);
  led_MIDDLE.setColor(LedSide::ALL, LedColor::RED);

  // Back off from obstacle.
  robotBase.move_tank(-SWITCH_BACK_PWM, 0);
  delay(SWITCH_BACK_MS);
  robotBase.stop(true);
  delay(20);

  // Turn away from touched side.
  if (ev == SwitchEvent::LEFT) {
    robotBase.move_tank(0, -SWITCH_TURN_PWM); // turn right
    delay(SWITCH_TURN_MS);
  } else if (ev == SwitchEvent::RIGHT) {
    robotBase.move_tank(0, SWITCH_TURN_PWM);  // turn left
    delay(SWITCH_TURN_MS);
  } else {
    robotBase.move_tank(0, SWITCH_TURN_PWM);  // both: default left turn
    delay((uint16_t)(SWITCH_TURN_MS * 1.6f));
  }

  robotBase.stop(true);
  led_MIDDLE.clear();
}

static TileType toTileType(FloorColor c) {
  switch (c) {
    case FloorColor::RED: return TileType::RED;
    case FloorColor::BLUE: return TileType::BLUE;
    case FloorColor::SILVER: return TileType::SILVER;
    case FloorColor::BLACK: return TileType::BLACK;
    default: return TileType::UNKNOWN;
  }
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Sistema iniciando...");
  delay(3000);

  if (!imub.begin(Wire, I2C_FREQUENCY)) {
    Serial.println("Aviso: BNO055 nao respondeu.");
  } else {
    imub.zeroYaw();
  }
 
  if (!tof.begin(Wire, I2C_FREQUENCY, 40)) {
    Serial.println("Erro ToF");
  }
  tof.setEmaAlpha(0.25f);

  floorSensor.begin(ADC_RESOLUTION);
  floorSensor.setDecisionChannels(0, 2); // C9 = idx0, canal auxiliar = idx3
  floorSensor.setMatchMode(ReflectanceMatchMode::RATIO);
  floorSensor.setRatioThresholdScale(1000.0f);
  floorSensor.setEmaAlpha(0.30f);
  floorSensor.setBlueThreshold({20, 80, 0, 0, 0.0f});
  floorSensor.setSilverThreshold({90, 130, 0, 0, 0.0f});
  floorSensor.setRedThreshold({135, 160, 0, 0, 0.0f});
  floorSensor.setBlackThreshold({230, 400, 0, 0, 0.0f});

  encFL.begin(true);
  encRR.begin(true);
  bumper.begin(true);

  
  robotBase.begin(PWM_RESOLUTION, PWM_FREQUENCY);
  robotBase.invertFL(true);
  robotBase.invertRR(true);
  robotBase.invertFR(true);
  robotBase.invertRL(true);
  led_LEFT.begin();
  led_MIDDLE.begin();
  led_RIGHT.begin();

  led_LEFT.setColor(LedSide::ALL, LedColor::WHITE);
  led_RIGHT.setColor(LedSide::ALL, LedColor::WHITE);


  servoKit.begin(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  //servoKit.setAngles(SERVO_CENTER_DEG, SERVO_LEFT_DROP_DEG, SERVO_RIGHT_DROP_DEG);
  servoKit.setTiming(SERVO_MOVE_DELAY_MS, SERVO_BETWEEN_KITS_MS);
  servoKit.setVictimKitMap(0, 1, 2); // Ajuste IDs conforme protocolo OpenMV.

  if (!camL.begin(I2C_FREQUENCY)) Serial.println("Aviso: OpenMV Left sem ACK.");
  if (!camR.begin(I2C_FREQUENCY)) Serial.println("Aviso: OpenMV Right sem ACK.");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Modo calibracao (8s): 't'=ToF, 'c'=Cor, 'a'=Ambos");
    char mode = 't';
    const uint32_t t0 = millis();
    while (millis() - t0 < 8000) {
      // if (Serial.available()) {
      //   const char c = (char)Serial.read();
      //   if (c == 't' || c == 'T' || c == 'c' || c == 'C' || c == 'a' || c == 'A') {
      //     mode = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
      //     break;
      //   }
      // }
      // delay(10);
    }

    if (mode == 't' || mode == 'a') {
      Serial.println("Calibrando ToF...");
      ToFCalibration::run(tof, led_MIDDLE);
    }
    // if (mode == 'c' || mode == 'a') {
    //   Serial.println("Calibrando cor...");
    //   ColorCalibration::run(floorSensor, BUTTON_PIN, 0, 3, true, 1000.0f);
    // }
  }
  ToFCalibration::load(tof);
  if (ColorCalibration::load(floorSensor)) {
    Serial.println("Calibracao de cor carregada da EEPROM.");
  } else {
    Serial.println("Sem calibracao de cor salva na EEPROM.");
  }

  controller.begin(0, 0, Heading::NORTH);
  controller.setTicksPerTile(ENCODER_TICKS_PER_TILE);



  Serial.println("Sistema pronto.");
  Serial.println("Deus abençoe o round.");
}

void loop() {
  led_MIDDLE.update();
  const bool buttonNow = digitalRead(BUTTON_PIN);
  if (buttonNow != lastButtonRead) {
    lastButtonRead = buttonNow;
    lastButtonEdgeMs = millis();
  }

  const bool debouncedPressed = (buttonNow == LOW) && ((millis() - lastButtonEdgeMs) > 30);
  const bool debouncedReleased = (buttonNow == HIGH) && ((millis() - lastButtonEdgeMs) > 30);

  if (debouncedPressed && !pauseByButton) {
    pauseByButton = true;
    robotBase.stop(true);
    pauseLedOn = false;
    pauseLedToggleMs = 0;
    if (controller.relocateToLastCheckpoint()) {
      Serial.println("Botao: navegacao interrompida, robo reposicionado no ultimo checkpoint.");
    } else {
      Serial.println("Botao: sem checkpoint salvo ainda, robo apenas pausado.");
    }
  }

  if (pauseByButton) {
    robotBase.stop(true);
    if (millis() - pauseLedToggleMs >= PAUSE_LED_INTERVAL_MS) {
      pauseLedToggleMs = millis();
      pauseLedOn = !pauseLedOn;
      if (pauseLedOn) led_MIDDLE.setColor(LedSide::ALL, LedColor::RED);
      else led_MIDDLE.clear();
    }
    if (debouncedReleased) {
      pauseByButton = false;
      led_MIDDLE.clear();
      Serial.println("Botao: navegacao retomada.");
    }
    return;
  }

  handleSwitchObstacle(bumper.poll());

  tof.update();

  static uint32_t t = 0;
  if (millis() - t < 50) return;
  t = millis();

  tof.snapshot(raw);
  scan.s = raw;

  floorSensor.read(1, 200);
  FloorColor floor = floorSensor.detect();

  //CameraData leftCam = camL.read(15, 0);
  //CameraData rightCam = camR.read(15, 0);
  PlannerInput in;
  in.sensedTile = toTileType(floor);
  in.frontFree = scan.minFront() > TOF_WALL_CLEAR_MM;
  in.leftFree = scan.minLeft() > TOF_WALL_CLEAR_MM;
  in.rightFree = scan.minRight() > TOF_WALL_CLEAR_MM;
  in.backFree = true;
  in.yawDeg = imub.getYaw360();
  in.encFlTicks = encFL.read();
  in.encRrTicks = encRR.read();
  in.tofLeftMm = scan.minLeft();
  in.tofRightMm = scan.minRight();
  //in.victimLeft = leftCam.vitima;
  //in.victimRight = rightCam.vitima;
  //in.victimConfLeft = leftCam.confianca;
  //in.victimConfRight = rightCam.confianca;
  in.nowMs = millis();

  PlannerOutput out = controller.update(in);

  const bool victimLeft = (in.victimConfLeft >= VICTIM_CONFIDENCE_MIN &&
                           servoKit.shouldDrop(in.victimLeft) &&
                           hasWallOnLeftVictimSide(scan));
  const bool victimRight = (in.victimConfRight >= VICTIM_CONFIDENCE_MIN &&
                            servoKit.shouldDrop(in.victimRight) &&
                            hasWallOnRightVictimSide(scan));
  const bool canActVictim = millis() >= nextVictimActionAtMs;
  bool victimActionTaken = false;
  if (canActVictim && (victimLeft || victimRight)) {
    const bool chooseLeft = victimLeft && (!victimRight || in.victimConfLeft >= in.victimConfRight);
    const LedSide side = chooseLeft ? LedSide::LEFT : LedSide::RIGHT;
    const uint8_t victimType = chooseLeft ? in.victimLeft : in.victimRight;
    robotBase.stop(true);
    led_MIDDLE.setColor(side, LedColor::YELLOW);
    servoKit.dropForVictim(chooseLeft ? ServoDropSide::LEFT : ServoDropSide::RIGHT, victimType);
    led_MIDDLE.clear();

    nextVictimActionAtMs = millis() + VICTIM_ACTION_COOLDOWN_MS;
    victimActionTaken = true;
  }

  if (victimActionTaken) {
    robotBase.stop(true);
  } else if (out.command == MotionCommand::DRIVE_CLOSED_LOOP || out.command == MotionCommand::RETREAT_ONE) {
    robotBase.move_tank(out.linearPwm, out.turnPwm);
  } else {
    robotBase.stop(true);
  }

  Serial.print("Tile=");
  Serial.print(RobotControl::toString(in.sensedTile));
  Serial.print(" | State=");
  Serial.print(RobotControl::toString(out.state));
  Serial.print(" | Phase=");
  Serial.print(RobotControl::toString(out.phase));
  Serial.print(" | Cmd=");
  Serial.print(RobotControl::toString(out.command));
  Serial.print(" | Nav=");
  Serial.print(RobotControl::toString(out.source));
  Serial.print(" | PWM(");
  Serial.print(out.linearPwm);
  Serial.print(",");
  Serial.print(out.turnPwm);
  Serial.print(") | Yaw=");
  Serial.print(in.yawDeg, 1);
  Serial.print(" | Enc(");
  Serial.print(in.encFlTicks);
  Serial.print(",");
  Serial.print(in.encRrTicks);
  Serial.print(") | Vict(");
  Serial.print(in.victimLeft);
  Serial.print("@");
  Serial.print(in.victimConfLeft);
  Serial.print(",");
  Serial.print(in.victimRight);
  Serial.print("@");
  Serial.print(in.victimConfRight);
  Serial.print(") | Pose=(");
  Serial.print(out.pose.x);
  Serial.print(",");
  Serial.print(out.pose.y);
  Serial.println(")");
}