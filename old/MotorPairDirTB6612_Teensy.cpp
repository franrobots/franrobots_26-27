#include "MotorPairDirTB6612_Teensy.h"

MotorPairDirTB6612::MotorPairDirTB6612(uint8_t inA, uint8_t inB, uint8_t pwm1, uint8_t pwm2)
: _inA(inA), _inB(inB), _pwm1(pwm1), _pwm2(pwm2),
  _pwmMax(4095), _deadband(0),
  _invertPair(false),
  _stopMode(StopMode::Coast),
  _pwmResolutionBits(12), _pwmFreqHz(20000)
{}

void MotorPairDirTB6612::begin(uint8_t pwmResolutionBits, uint32_t pwmFreqHz) {
  _pwmResolutionBits = pwmResolutionBits;
  _pwmFreqHz = pwmFreqHz;

  pinMode(_inA, OUTPUT);
  pinMode(_inB, OUTPUT);

  pinMode(_pwm1, OUTPUT);
  pinMode(_pwm2, OUTPUT);

  analogWriteResolution(_pwmResolutionBits);

  analogWriteFrequency(_pwm1, _pwmFreqHz);
  analogWriteFrequency(_pwm2, _pwmFreqHz);

  _pwmMax = (uint16_t)((1u << _pwmResolutionBits) - 1u);

  stop(_stopMode);
}

void MotorPairDirTB6612::setInvertPair(bool invert) { _invertPair = invert; }
void MotorPairDirTB6612::setDeadband(uint16_t deadbandPwm) { _deadband = deadbandPwm; }
void MotorPairDirTB6612::setMaxPwm(uint16_t maxPwm) { _pwmMax = maxPwm; }
void MotorPairDirTB6612::setStopMode(StopMode mode) { _stopMode = mode; }

uint16_t MotorPairDirTB6612::applyDeadband(uint16_t mag) const {
  if (mag == 0) return 0;
  if (mag > _pwmMax) mag = _pwmMax;
  if (_deadband && mag < _deadband) return _deadband;
  return mag;
}

void MotorPairDirTB6612::writePWM(uint8_t pin, uint16_t value) {
  if (value > _pwmMax) value = _pwmMax;
  analogWrite(pin, value);
}

void MotorPairDirTB6612::setDirectionForward()  { digitalWrite(_inA, HIGH); digitalWrite(_inB, LOW); }
void MotorPairDirTB6612::setDirectionBackward() { digitalWrite(_inA, LOW);  digitalWrite(_inB, HIGH); }
void MotorPairDirTB6612::setDirectionCoast()    { digitalWrite(_inA, LOW);  digitalWrite(_inB, LOW); }
void MotorPairDirTB6612::setDirectionBrake()    { digitalWrite(_inA, HIGH); digitalWrite(_inB, HIGH); }

void MotorPairDirTB6612::drive(int16_t cmd, uint16_t pwmMotor1, uint16_t pwmMotor2) {
  if (cmd == 0) {
    stop(_stopMode);
    return;
  }

  if (_invertPair) cmd = -cmd;

  if (cmd > 0) setDirectionForward();
  else         setDirectionBackward();

  uint16_t m1 = applyDeadband(pwmMotor1);
  uint16_t m2 = applyDeadband(pwmMotor2);

  writePWM(_pwm1, m1);
  writePWM(_pwm2, m2);
}

void MotorPairDirTB6612::drive(int16_t cmd) {
  uint16_t mag = (uint16_t)abs(cmd);
  if (mag > _pwmMax) mag = _pwmMax;
  drive(cmd, mag, mag);
}

void MotorPairDirTB6612::stop(StopMode mode) {
  _stopMode = mode;

  if (mode == StopMode::Coast) {
    setDirectionCoast();
    writePWM(_pwm1, 0);
    writePWM(_pwm2, 0);
  } else {
    setDirectionBrake();
    writePWM(_pwm1, 0);
    writePWM(_pwm2, 0);
  }
}

void MotorPairDirTB6612::stopCoast() { stop(StopMode::Coast); }
void MotorPairDirTB6612::stopBrake() { stop(StopMode::Brake); }
