#include <Arduino.h>
#include "VL53Mux12_FRAN.h"
#include "Scan360.h"
#include "ToFCalibration.h"

constexpr uint8_t TCA_A = 0x70;
constexpr uint8_t TCA_B = 0x71;
constexpr uint8_t BUTTON_PIN = 23;

VL53Mux12_FRAN tof(TCA_A, TCA_B);
Scan360 scan;
ScanToF12 raw;

void setup() {
  Serial.begin(115200);
  delay(200);

  if (!tof.begin(Wire, 400000, 40)) {
    Serial.println("Erro ToF");
    while (1);
  }

  tof.setEmaAlpha(0.25f);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Modo calibracao...");
    ToFCalibration::run(tof, BUTTON_PIN);
  }

  ToFCalibration::load(tof);

  Serial.println("Sistema pronto.");
}

void loop() {
  tof.update();

  static uint32_t t = 0;
  if (millis() - t > 100) {
    t = millis();

    tof.snapshot(raw);
    scan.s = raw;

    Serial.print("FrontMin=");
    Serial.print(scan.minFront());
    Serial.print(" | Skew=");
    Serial.print(scan.frontSkew());
    Serial.print(" | CorrErr=");
    Serial.println(scan.corridorError());
  }
}