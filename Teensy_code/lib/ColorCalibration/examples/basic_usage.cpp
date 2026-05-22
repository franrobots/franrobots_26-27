#include <Arduino.h>
#include "ReflectancePlate.h"
#include "ColorCalibration.h"

const uint8_t pins[] = {A0, A1, A2, A3};
ReflectancePlate plate(pins, 4);

void setup() {
  Serial.begin(115200);
  pinMode(23, INPUT_PULLUP); // button

  plate.begin(12);
  plate.setDecisionChannels(0, 3);
  plate.setMatchMode(ReflectanceMatchMode::RATIO);
  plate.setRatioThresholdScale(1000.0f);

  if (digitalRead(23) == LOW) {
    ColorCalibration::run(plate, 23, 0, 3, true, 1000.0f);
  }
  ColorCalibration::load(plate);
}

void loop() {
  plate.read();
  Serial.println(plate.toString(plate.detect()));
  delay(100);
}
