#include <Arduino.h>
#include "VL53Mux12_FRAN.h"

// Ajuste os endereços reais dos seus TCAs
static constexpr uint8_t TCA_A = 0x70;
static constexpr uint8_t TCA_B = 0x71;

VL53Mux12_FRAN tof(TCA_A, TCA_B);
ScanToF12 scan;

void setup() {
  Serial.begin(115200);
  delay(200);

  if (!tof.begin(Wire, 400000, 50)) { // 50ms por sensor
    Serial.println("ERRO: falha ao iniciar VL53 + TCA");
    while (1) delay(10);
  }

  Serial.println("TOF ready (non-blocking RR).");
}

void loop() {
  // não bloqueia: atualiza no máximo 1 sensor por chamada
  tof.update();

  // imprime a cada 100ms sem travar o controle do robô
  static uint32_t tPrint = 0;
  if (millis() - tPrint >= 100) {
    tPrint = millis();

    tof.snapshot(scan);

    Serial.print("Front: FL=");
    Serial.print(scan.mm[(uint8_t)ToFId::FL]);
    Serial.print(" FC=");
    Serial.print(scan.mm[(uint8_t)ToFId::FC]);
    Serial.print(" FR=");
    Serial.print(scan.mm[(uint8_t)ToFId::FR]);

    Serial.print(" | Left: LF=");
    Serial.print(scan.mm[(uint8_t)ToFId::LF]);
    Serial.print(" LC=");
    Serial.print(scan.mm[(uint8_t)ToFId::LC]);
    Serial.print(" LB=");
    Serial.print(scan.mm[(uint8_t)ToFId::LB]);

    Serial.print(" | Right: RF=");
    Serial.print(scan.mm[(uint8_t)ToFId::RF]);
    Serial.print(" RC=");
    Serial.print(scan.mm[(uint8_t)ToFId::RC]);
    Serial.print(" RB=");
    Serial.println(scan.mm[(uint8_t)ToFId::RB]);

        Serial.print(" | BACK: BL=");
    Serial.print(scan.mm[(uint8_t)ToFId::BL]);
    Serial.print(" BC=");
    Serial.print(scan.mm[(uint8_t)ToFId::BC]);
    Serial.print(" BR=");
    Serial.println(scan.mm[(uint8_t)ToFId::BR]);
  }
}