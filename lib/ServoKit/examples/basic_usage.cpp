#include <Arduino.h>
#include "ServoKit.h"

ServoKit kit;

void setup() {
  Serial.begin(115200);

  kit.begin(10, 1000, 2000);
  kit.setAngles(90, 35, 145);
  kit.setTiming(250, 220);
  kit.setVictimKitMap(0, 1, 2);
}

void loop() {
  // Exemplo: vítima tipo 2 no lado esquerdo -> 2 kits.
  kit.dropKits(ServoDropSide::LEFT, 2);
  delay(2000);
}
