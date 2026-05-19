#include "ToFCalibration.h"

static constexpr uint32_t MAGIC = 0xDEADBEEF;

struct Blob {
  uint32_t magic;
  int16_t offset[(uint8_t)ToFId::COUNT];
};

void ToFCalibration::run(VL53Mux12_FRAN& tof, uint8_t buttonPin) {
  pinMode(buttonPin, INPUT_PULLUP);

  Blob blob;
  blob.magic = MAGIC;

  for (uint8_t i = 0; i < (uint8_t)ToFId::COUNT; i++) {
    Serial.print("Encoste sensor ");
    Serial.print(i);
    Serial.println(" na parede e aperte botao...");

    while (digitalRead(buttonPin) == HIGH) delay(5);
    delay(200);

    tof.readSensor((ToFId)i);
    uint16_t raw = tof.getRaw((ToFId)i);
    blob.offset[i] = raw;

    Serial.print("Offset salvo: ");
    Serial.println(raw);

    while (digitalRead(buttonPin) == LOW) delay(5);
  }

  EEPROM.put(0, blob);
  Serial.println("Calibracao salva.");
}

void ToFCalibration::load(VL53Mux12_FRAN& tof) {
  Blob blob;
  EEPROM.get(0, blob);
  if (blob.magic != MAGIC) return;

  for (uint8_t i = 0; i < (uint8_t)ToFId::COUNT; i++) {
    tof.setOffset((ToFId)i, blob.offset[i]);
  }
}
