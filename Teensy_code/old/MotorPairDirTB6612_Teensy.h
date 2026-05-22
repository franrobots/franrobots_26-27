#pragma once
#include <Arduino.h>

class MotorPairDirTB6612 {
public:
  enum class StopMode : uint8_t { Coast, Brake };

  // inA/inB = direção compartilhada do par
  // pwm1/pwm2 = PWMs individuais de cada motor do par
  MotorPairDirTB6612(uint8_t inA, uint8_t inB, uint8_t pwm1, uint8_t pwm2);

  void begin(uint8_t pwmResolutionBits = 12, uint32_t pwmFreqHz = 20000);

  void setInvertPair(bool invert);
  void setDeadband(uint16_t deadbandPwm);
  void setMaxPwm(uint16_t maxPwm);
  void setStopMode(StopMode mode);

  // cmd define direção do par (sinal) e intensidade base
  // pwm1/pwm2 são magnitudes já prontas (0..pwmMax)
  void drive(int16_t cmd, uint16_t pwmMotor1, uint16_t pwmMotor2);

  // atalho: mesma magnitude pros dois
  void drive(int16_t cmd);

  void stop(StopMode mode);
  void stopCoast();
  void stopBrake();

private:
  void setDirectionForward();
  void setDirectionBackward();
  void setDirectionCoast();
  void setDirectionBrake();

  void writePWM(uint8_t pin, uint16_t value);
  uint16_t applyDeadband(uint16_t mag) const;

  uint8_t _inA, _inB;
  uint8_t _pwm1, _pwm2;

  uint16_t _pwmMax;
  uint16_t _deadband;

  bool _invertPair;
  StopMode _stopMode;

  uint8_t _pwmResolutionBits;
  uint32_t _pwmFreqHz;
};
