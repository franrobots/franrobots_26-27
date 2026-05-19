#include <Arduino.h>
#include "BNO055_FranRobots.h"

BNO055_FranRobots bno(0x29);

void setup() {
  Serial.begin(115200);
  if (!bno.begin(Wire, 400000)) {
    Serial.println("BNO055 fail");
    while (1) {}
  }
  bno.zeroYaw();
}

void loop() {
  Serial.println(bno.getYaw360(), 1);
  delay(100);
}
