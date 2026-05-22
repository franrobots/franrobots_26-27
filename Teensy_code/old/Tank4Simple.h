#pragma once
#include <Arduino.h>

class Tank4Simple {
public:
  Tank4Simple(uint8_t in00a, uint8_t in00b,
              uint8_t in01a, uint8_t in01b,
              uint8_t pwm_m1, uint8_t pwm_m2,
              uint8_t pwm_m3, uint8_t pwm_m4);

  void begin(uint8_t resolutionBits = 12, uint32_t freqHz = 20000);

  void setMotorGains(float g1, float g2, float g3, float g4);
  void setCorrectionFn(int (*fn)(int));

  // ✅ novo
  void setDeadband(uint16_t db);

  void moveTank(int left_value, int right_value, bool correctionFlag);
  void stop(bool brake = true);

  uint16_t maxPwm() const { return _maxPwm; }

private:
  void applySide(int value, uint8_t inA, uint8_t inB, uint8_t pwm1, uint8_t pwm2,
                 float g1, float g2);

  uint16_t applyDeadband(uint16_t mag) const;
  uint16_t scaleAndClamp(uint16_t mag, float gain) const;

  uint8_t _in00a, _in00b, _in01a, _in01b;
  uint8_t _pwm1, _pwm2, _pwm3, _pwm4;

  uint16_t _maxPwm;
  uint16_t _deadband; // ✅ novo

  float _g1, _g2, _g3, _g4;

  int (*_correctionFn)(int);
};
