#include <Arduino.h>
#include "Scan360.h"

Scan360 scan;

void setup() {
  Serial.begin(115200);
  // Exemplo artificial
  for (uint8_t i = 0; i < (uint8_t)ToFId::COUNT; i++) {
    scan.s.mm[i] = 300;
    scan.s.ok[i] = true;
    scan.s.age_ms[i] = 0;
  }
}

void loop() {
  Serial.print("front=");
  Serial.print(scan.minFront());
  Serial.print(" err=");
  Serial.println(scan.corridorError());
  delay(100);
}
