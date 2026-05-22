#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "ReflectancePlate.h"

class ColorCalibration {
public:
  static void run(ReflectancePlate& plate,
                  uint8_t buttonPin,
                  uint8_t c9Idx = 0,
                  uint8_t auxIdx = 3,
                  bool ratioMode = true,
                  float ratioScale = 1000.0f);

  static bool load(ReflectancePlate& plate);
};
