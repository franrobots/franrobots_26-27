#pragma once 
#include "Arduino.h"
#include "config.h"

class motor {
  public:
    motor(uint8_t pinAin1, uint8_t pinAin2, uint16_t pinPWM);
    void begin(uint8_t resolutionBits =12, uint32_t freqHZ = 20000);
    uint16_t applyPWM(int16_t valuePWM);
    void stopMotor(bool brake = true);
    void setDeadband(uint16_t db);
    void setCorrectionFn(int (*fn)(int));
    void setMotorGain(float gain);
    uint8_t _pinAin1;
    uint8_t _pinAin2;

  private:

    uint16_t _pinPWM;  
    uint16_t _valuePWM; 
    uint16_t _maxPwm; 
    uint16_t _deadband;
    float _gain;

    int (*_correctionFn)(int);
    uint16_t applyDeadband(uint16_t mag) const;
    uint16_t scaleAndClamp(uint16_t mag, float gain) const;
};