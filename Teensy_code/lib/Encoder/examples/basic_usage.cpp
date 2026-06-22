#include <Arduino.h>
#include "Encoder.h"

Encoder enc1(20, 21);
Encoder enc2(22, 23);

void setup() {
  Serial.begin(115200);
  enc1.begin(true);
  enc2.begin(true);
}

void loop() {
  Serial.print("1=");
  Serial.print(enc1.read());
  Serial.print(" 2=");
  Serial.println(enc2.read());
  delay(100);
}
