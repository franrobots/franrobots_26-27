#include "Motor.h"

static inline int clamp_int(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

motor::motor(uint8_t pinAin1, uint8_t pinAin2)
    : _pinAin1(pinAin1),
      _pinAin2(pinAin2),
      _maxPwm(4095),
      _deadband(0),
      _gain(1.0f),
      _correctionFn(nullptr)
{
}

void motor::begin(uint8_t resolutionBits, uint32_t freqHZ)
{
    pinMode(_pinAin1, OUTPUT);
    pinMode(_pinAin2, OUTPUT);
    // pinMode(_pinPWM, OUTPUT);

    analogWriteResolution(resolutionBits);
    // analogWriteFrequency(freqHZ);
    analogWriteFrequency(_pinAin1,freqHZ);
    analogWriteFrequency(_pinAin2,freqHZ);
    // analogWriteFrequency(_pinPWM,freqHZ);

    _maxPwm = (uint16_t)((1u << resolutionBits) - 1u);

    stopMotor(true);
}

uint16_t motor::applyPWM(int16_t valuePWM) {
    valuePWM = clamp_int(valuePWM, -(int)_maxPwm, (int)_maxPwm);
    if (_correctionFn) valuePWM = _correctionFn(valuePWM);

    uint16_t mag = scaleAndClamp(applyDeadband((uint16_t)abs((int)valuePWM)), _gain);

    if (valuePWM == 0) {
        analogWrite(_pinAin1, 0);
        analogWrite(_pinAin2, 0);
        _valuePWM = 0;
        return 0;
    }

    if (valuePWM < 0) {
        analogWrite(_pinAin2, 0);
        analogWrite(_pinAin1, mag);
    } else {
        analogWrite(_pinAin1, 0);
        analogWrite(_pinAin2, mag);
    }
    return mag;
}

void motor::stopMotor(bool brake) {
    analogWrite(_pinAin1, 0);
    analogWrite(_pinAin2, 0);
    _valuePWM = 0;
}

void motor::setDeadband(uint16_t db)
{
    if (db > _maxPwm)
        db = _maxPwm;
    _deadband = db;
}

void motor::setCorrectionFn(int (*fn)(int))
{
    _correctionFn = fn;
}

void motor::setMotorGain(float gain)
{
    _gain = (gain <= 0.05f) ? 0.05f : gain;
}

uint16_t motor::applyDeadband(uint16_t mag) const
{
    if (mag == 0)
        return 0;
    if (_deadband && mag < _deadband)
        return _deadband;
    return mag;
}

uint16_t motor::scaleAndClamp(uint16_t mag, float gain) const
{
    float v = (float)mag * gain;
    if (v < 0.0f)
        v = 0.0f;
    if (v > (float)_maxPwm)
        v = (float)_maxPwm;
    return (uint16_t)lroundf(v);
}

