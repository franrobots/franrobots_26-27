#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "VL53Mux12_FRAN.h"
#include "LedStrip.h"

class ToFCalibration {
public:
  static void run(VL53Mux12_FRAN& tof, LedStrip& ledStrip);
  static void load(VL53Mux12_FRAN& tof);
};