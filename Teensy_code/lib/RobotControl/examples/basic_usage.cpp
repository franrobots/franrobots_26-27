#include <Arduino.h>
#include "RobotControl.h"

RobotControl ctrl;

void setup() {
  Serial.begin(115200);
  ctrl.begin(0, 0, Heading::NORTH);
  ctrl.setTicksPerTile(860);
}

void loop() {
  PlannerInput in;
  in.frontFree = true;
  in.leftFree = true;
  in.rightFree = false;
  in.backFree = true;
  in.yawDeg = 0.0f;
  in.encFlTicks = 0;
  in.encRrTicks = 0;
  in.nowMs = millis();

  PlannerOutput out = ctrl.update(in);
  Serial.print("cmd=");
  Serial.print(RobotControl::toString(out.command));
  Serial.print(" nav=");
  Serial.println(RobotControl::toString(out.source));
  delay(100);
}
