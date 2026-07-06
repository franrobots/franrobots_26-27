#include "ToFCalibration.h"

static constexpr uint32_t MAGIC = 0xDEADBEEF; 
static constexpr uint8_t X_READINGS = 50;     
struct Blob { //
  uint32_t magic; //
  int16_t offset[(uint8_t)ToFId::COUNT]; 
};

struct SideConfig {
  uint8_t ledIndex;        
  uint8_t sensorCount;      
  uint8_t sensorIndices[4]; 
};

static const SideConfig LADOS[] = {
  { 0, 4, {0, 1, 2, 3} },   
  { 1, 4, {4, 5, 6, 7} },   
  { 2, 4, {8, 9, 10, 11} }  
};
constexpr uint8_t NUM_LADOS = sizeof(LADOS) / sizeof(LADOS[0]);

void ToFCalibration::run(VL53Mux12_FRAN& tof, uint8_t buttonPin, LedStrip& ledStrip) {
  pinMode(buttonPin, INPUT_PULLUP); 

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

    while (!sideCalibrated) {
      uint32_t agora = millis();
      
      if (agora - lastBlinkMs >= 250) {
        lastBlinkMs = agora;
        ledOn = !ledOn;
        ledStrip.clear();
        if (ledOn) {
          //ledStrip.setPixel(lado.ledIndex, LedColor::BLUE); 
        }
        ledStrip.update();
      }

      uint16_t menorDistancia = 9999;
      for (uint8_t s = 0; s < lado.sensorCount; s++) {
        uint8_t idxSensor = lado.sensorIndices[s];
        tof.readSensor((ToFId)idxSensor);
        uint16_t raw = tof.getRaw((ToFId)idxSensor);
        
        if (raw < menorDistancia && raw > 0) { 
          menorDistancia = raw;
        }
      }

      if (menorDistancia < 100) {
        ledStrip.clear();
        //ledStrip.setPixel(lado.ledIndex, LedColor::BLUE);
        ledStrip.update();

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
        ledStrip.update();
        sideCalibrated = true; 
      }
      
      delay(10); 
    }
  }

  for (uint8_t b = 0; b < 4; b++) {
    ledStrip.setColor(LedSide::ALL, LedColor::GREEN);
    ledStrip.update();
    delay(300);
    ledStrip.clear();
    ledStrip.update();
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
