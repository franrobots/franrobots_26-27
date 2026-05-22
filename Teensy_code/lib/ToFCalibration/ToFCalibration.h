#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "VL53Mux12_FRAN.h"

class ToFCalibration {
public:
  static void run(VL53Mux12_FRAN& tof, uint8_t buttonPin);
  static void load(VL53Mux12_FRAN& tof);
};