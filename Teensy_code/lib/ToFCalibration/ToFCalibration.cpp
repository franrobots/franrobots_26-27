#include "ToFCalibration.h"

static constexpr uint32_t MAGIC = 0xDEADBEEF; 
static constexpr uint8_t X_READINGS = 50;     

struct Blob { 
  uint32_t magic; 
  int16_t offset[(uint8_t)ToFId::COUNT]; 
};

struct SideConfig {
  uint8_t ledIndex;        
  uint8_t sensorCount;      
  uint8_t sensorIndices[4]; 
};

// Mapeamento estrito usando os enums do ToFId
static const SideConfig LADOS[] = {
  { 0, 3, { (uint8_t)ToFId::LF, (uint8_t)ToFId::LC, (uint8_t)ToFId::LB } }, // Lado 0 (LED 0): ESQUERDA
  { 1, 3, { (uint8_t)ToFId::FR, (uint8_t)ToFId::FC, (uint8_t)ToFId::FL } }, // Lado 1 (LED 1): FRENTE
  { 2, 3, { (uint8_t)ToFId::RB, (uint8_t)ToFId::RC, (uint8_t)ToFId::RF } }, // Lado 2 (LED 2): DIREITA
  { 3, 2, { (uint8_t)ToFId::BR, (uint8_t)ToFId::BL } }                    // Lado 3 (LED 3): TRÁS
};

constexpr uint8_t NUM_LADOS = sizeof(LADOS) / sizeof(LADOS[0]);

void ToFCalibration::run(VL53Mux12_FRAN& tof, LedStrip& ledStrip) {
  Blob blob; 
  EEPROM.get(0, blob); 
  if (blob.magic != MAGIC) {
    blob.magic = MAGIC;
    for (uint8_t i = 0; i < (uint8_t)ToFId::COUNT; i++) {
      blob.offset[i] = 0;
    }
  }

  for (uint8_t l = 0; l < NUM_LADOS; l++) {
    const SideConfig& lado = LADOS[l];
    
    unsigned long lastBlinkMs = 0;
    bool ledOn = false;
    bool sideCalibrated = false;
    uint8_t leiturasConfirmadas = 0;

    while (!sideCalibrated) {
      uint32_t agora = millis();
      
      if (agora - lastBlinkMs >= 250) {
        lastBlinkMs = agora;
        ledOn = !ledOn;
        ledStrip.clear();
        if (ledOn) {
          ledStrip.setpixel(LedSide::ALL, lado.ledIndex, LedColor::WHITE);
        }
      }

      uint16_t menorDistancia = 9999;
      for (uint8_t s = 0; s < lado.sensorCount; s++) {
        uint8_t idxSensor = lado.sensorIndices[s];
        tof.readSensor((ToFId)idxSensor);
        uint16_t raw = tof.getRaw((ToFId)idxSensor);
        
        if (raw < menorDistancia && raw > 15 && raw < 2000) { 
          menorDistancia = raw;
        }
      }

      if (menorDistancia < 60) {
        leiturasConfirmadas++;
      } else {
        leiturasConfirmadas = 0;
      }

      if (leiturasConfirmadas >= 3) {
        ledStrip.clear();
        ledStrip.setpixel(LedSide::ALL, lado.ledIndex, LedColor::WHITE);

        uint32_t somaLeituras[4] = {0};

        for (uint8_t r = 0; r < X_READINGS; r++) {
          for (uint8_t s = 0; s < lado.sensorCount; s++) {
            uint8_t idxSensor = lado.sensorIndices[s];
            tof.readSensor((ToFId)idxSensor);
            somaLeituras[s] += tof.getRaw((ToFId)idxSensor);
          }
          delay(15); 
        }

        for (uint8_t s = 0; s < lado.sensorCount; s++) {
          uint8_t idxSensor = lado.sensorIndices[s];
          uint16_t media = somaLeituras[s] / X_READINGS;
          
          int16_t valorCalibrado = (int16_t)(media * 0.80f); 
          blob.offset[idxSensor] = valorCalibrado;
        }

        EEPROM.put(0, blob); 
        ledStrip.clear();
        sideCalibrated = true; 
      }
      
      delay(10); 
    }
  }

  for (uint8_t b = 0; b < 4; b++) {
    ledStrip.setColor(LedSide::ALL, LedColor::GREEN);
    delay(300);
    ledStrip.clear();
    delay(300);
  }
}

void ToFCalibration::load(VL53Mux12_FRAN & tof){
  Blob blob;
  EEPROM.get(0, blob);
  if (blob.magic != MAGIC)
    return;
  for (uint8_t i = 0; i < (uint8_t)ToFId::COUNT; i++){
    tof.setOffset((ToFId)i, blob.offset[i]);
  }  
}