#include <Arduino.h>
#include <Pot.h>
#include <Led.h>
#include "config.h"

pot myPot(POT_PIN);
led myLed(LED_PIN);

void setup() {
  Serial.begin(115200);
  myPot.begin();
  myLed.begin();
}


// void loop() {
//   uint16_t raw = myPot.read();
//   uint32_t filt = myPot.readFiltered();

//   Serial.print("Potentiometer value: ");
//   Serial.println(raw);

//   Serial.print("Potentiometer value (filtered): ");
//   Serial.println(filt);

//   myLed.pwmApply(raw >> (12 - LED_RESOLUTION));
// }

// void loop() {
//  myLed.blick(1000);
// }


void loop() {
  uint16_t raw = myPot.read();
  myPot.updateFastFilter(raw);
  uint16_t filt = myPot.readFastFiltered();

  if (raw <= 4)     filt = 0;
  if (raw >= 4087)  filt = 4095;

  myLed.pwmApply(filt >> (12 - LED_RESOLUTION));

  static uint32_t t = 0;
  if (millis() - t > 200) {
    t = millis();
    Serial.print("raw=");
    Serial.print(raw);
    Serial.print(" filt=");
    Serial.println(filt);
  }
}
