#include <Arduino.h>
#include "VL53Mux12_FRAN.h"

VL53Mux12_FRAN tof(0x70, 0x71);

void setup() {
  Serial.begin(115200);
  if (!tof.begin(Wire, 400000)) {
    Serial.println("VL53L0A fail");
  }
}

void loop() {
  tof.readAll();
  ScanToF12 scan;
  tof.snapshot(scan);
  tof.printAll(scan.mm);
}
