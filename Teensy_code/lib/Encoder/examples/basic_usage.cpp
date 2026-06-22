#include <Arduino.h>
#include "Encoder.h"

Encoder encA(20);
Encoder encB(22);

void setup() {
  Serial.begin(115200);
  encA.begin(true);
  encB.begin(true);
}

void loop() {
  Serial.print("A=");
  Serial.print(encA.read());
  Serial.print(" B=");
  Serial.println(encB.read());
  delay(100);
}
