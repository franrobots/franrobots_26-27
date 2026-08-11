#include <Arduino.h>
#include "VL53Mux12_FRAN.h"
#include "ToFCalibration.h"
#include "LedStrip.h"
#include "config.h"

VL53Mux12_FRAN tof(0x70, 0x71);
LedStrip ledStrip(LED_MIDDLE, 7); // Assuming 12 LEDs on the strip

void setup() {
  Serial.begin(115200);
  pinMode(23, INPUT_PULLUP);

  if (!tof.begin(Wire, 400000, 40)) {
    while (1) {}
  }

  if (digitalRead(23) == LOW) {
    ToFCalibration::run(tof, ledStrip);
  }
  ToFCalibration::load(tof);
}

void loop() {
  tof.update();
  delay(20);
}
