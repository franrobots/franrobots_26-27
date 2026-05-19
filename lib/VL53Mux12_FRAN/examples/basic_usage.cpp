#include <Arduino.h>
#include "VL53Mux12_FRAN.h"

VL53Mux12_FRAN tof(0x70, 0x71);
ScanToF12 snap;

void setup() {
  Serial.begin(115200);
  if (!tof.begin(Wire, 400000, 40)) {
    Serial.println("ToF fail");
    tof.getSensorOk();
    while (1) {}
  }
  tof.setEmaAlpha(0.25f);
}

void loop() {
  tof.readAll();
  tof.snapshot(snap);

  Serial.print("FC=");
  Serial.println(snap.mm[(uint8_t)ToFId::FC]);
  delay(100);
}
