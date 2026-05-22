#include <Arduino.h>
#include "ReflectancePlate.h"

const uint8_t pins[] = {A0, A1, A2, A3}; // C9, B, R, G (exemplo)
ReflectancePlate plate(pins, 4);

void setup() {
  Serial.begin(115200);
  plate.begin(12);
  plate.setEmaAlpha(0.30f);
  plate.setDecisionChannels(0, 3);
  plate.setMatchMode(ReflectanceMatchMode::RATIO);
  plate.setRatioThresholdScale(1000.0f);

  plate.setBlackThreshold({200, 450, 0, 0, 0.0f});
  plate.setBlueThreshold({500, 900, 0, 0, 0.0f});
  plate.setRedThreshold({901, 1300, 0, 0, 0.0f});
  plate.setSilverThreshold({1301, 2200, 0, 0, 0.0f});
}

void loop() {
  plate.read(1, 200);
  Serial.print("C9=");
  Serial.print(plate.c9());
  Serial.print(" color=");
  Serial.println(plate.toString(plate.detect()));
  delay(80);
}
