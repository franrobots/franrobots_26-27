#include <Arduino.h>
#include "Encoder.h"

Encoder enc1(23, 21);
Encoder enc2(26, 20);

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
