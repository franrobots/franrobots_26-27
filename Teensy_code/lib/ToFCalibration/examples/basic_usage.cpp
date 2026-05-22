#include <Arduino.h>
#include "VL53Mux12_FRAN.h"
#include "ToFCalibration.h"

VL53Mux12_FRAN tof(0x70, 0x71);

void setup() {
  Serial.begin(115200);
  pinMode(23, INPUT_PULLUP);

  if (!tof.begin(Wire, 400000, 40)) {
    while (1) {}
  }

  if (digitalRead(23) == LOW) {
    ToFCalibration::run(tof, 23);
  }
  ToFCalibration::load(tof);
}

void loop() {
  tof.update();
  delay(20);
}
