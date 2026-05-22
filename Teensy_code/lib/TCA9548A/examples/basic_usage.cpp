#include <Arduino.h>
#include "TCA9548A.h"

TCA9548A mux(0x70);

void setup() {
  Serial.begin(115200);
  if (!mux.begin(Wire)) {
    Serial.println("MUX fail");
  }
  if (!mux.select(0)) {
    Serial.println("Canal 0 sem ACK");
  }
}

void loop() {
  // Use sensores do canal 0 aqui.
  delay(100);
}
