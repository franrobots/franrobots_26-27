#pragma once
#include <Arduino.h>
#include "MotorPairDirTB6612_Teensy.h"

class Drive4_PairedDir {
public:
  // pairLeft: M1/M2 | pairRight: M3/M4
  Drive4_PairedDir(MotorPairDirTB6612& pairLeft, MotorPairDirTB6612& pairRight);

  void begin(uint8_t pwmResolutionBits = 12, uint32_t pwmFreqHz = 20000);

  void setCorrectionFn(int (*fn)(int));

  // Ganho por motor (trim)
  // Ex: (1.00, 0.95, 1.08, 1.00)
  void setMotorGains(float g1, float g2, float g3, float g4);

  void setStopModeAll(MotorPairDirTB6612::StopMode mode);
  void setDeadbandAll(uint16_t db);
  void setMaxPwmAll(uint16_t maxPwm);

  void moveTank(int left_value, int right_value, bool correctionFlag);

  void stopAll(MotorPairDirTB6612::StopMode mode = MotorPairDirTB6612::StopMode::Coast);

private:
  uint16_t scaleMag(int baseCmdAbs, float gain, uint16_t pwmMax) const;

  MotorPairDirTB6612& _left;   // M1/M2
  MotorPairDirTB6612& _right;  // M3/M4

  int (*_correctionFn)(int);

  float _g1, _g2, _g3, _g4;
  uint16_t _pwmMaxCached;
};
