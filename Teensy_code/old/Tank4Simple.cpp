#include "Tank4Simple.h"

static inline int clamp_int(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

Tank4Simple::Tank4Simple(uint8_t in00a, uint8_t in00b,
                         uint8_t in01a, uint8_t in01b,
                         uint8_t pwm_m1, uint8_t pwm_m2,
                         uint8_t pwm_m3, uint8_t pwm_m4)
: _in00a(in00a), _in00b(in00b), _in01a(in01a), _in01b(in01b),
  _pwm1(pwm_m1), _pwm2(pwm_m2), _pwm3(pwm_m3), _pwm4(pwm_m4),
  _maxPwm(4095),
  _deadband(0),                 // ✅ novo
  _g1(1.0f), _g2(1.0f), _g3(1.0f), _g4(1.0f),
  _correctionFn(nullptr)
{}

void Tank4Simple::begin(uint8_t resolutionBits, uint32_t freqHz) {
  pinMode(_in00a, OUTPUT);
  pinMode(_in00b, OUTPUT);
  pinMode(_in01a, OUTPUT);
  pinMode(_in01b, OUTPUT);

  pinMode(_pwm1, OUTPUT);
  pinMode(_pwm2, OUTPUT);
  pinMode(_pwm3, OUTPUT);
  pinMode(_pwm4, OUTPUT);

  analogWriteResolution(resolutionBits);

  analogWriteFrequency(_pwm1, freqHz);
  analogWriteFrequency(_pwm2, freqHz);
  analogWriteFrequency(_pwm3, freqHz);
  analogWriteFrequency(_pwm4, freqHz);

  _maxPwm = (uint16_t)((1u << resolutionBits) - 1u);

  stop(true);
}

void Tank4Simple::setMotorGains(float g1, float g2, float g3, float g4) {
  _g1 = (g1 <= 0.05f) ? 0.05f : g1;
  _g2 = (g2 <= 0.05f) ? 0.05f : g2;
  _g3 = (g3 <= 0.05f) ? 0.05f : g3;
  _g4 = (g4 <= 0.05f) ? 0.05f : g4;
}

void Tank4Simple::setCorrectionFn(int (*fn)(int)) {
  _correctionFn = fn;
}

// ✅ novo
void Tank4Simple::setDeadband(uint16_t db) {
  if (db > _maxPwm) db = _maxPwm;
  _deadband = db;
}

uint16_t Tank4Simple::applyDeadband(uint16_t mag) const {
  if (mag == 0) return 0;
  if (_deadband && mag < _deadband) return _deadband;
  return mag;
}

uint16_t Tank4Simple::scaleAndClamp(uint16_t mag, float gain) const {
  float v = (float)mag * gain;
  if (v < 0.0f) v = 0.0f;
  if (v > (float)_maxPwm) v = (float)_maxPwm;
  return (uint16_t)lroundf(v);
}

void Tank4Simple::applySide(int value, uint8_t inA, uint8_t inB, uint8_t pwm1, uint8_t pwm2,
                            float g1, float g2) {
  // Zera PWM antes de trocar direção (proteção)
  analogWrite(pwm1, 0);
  analogWrite(pwm2, 0);

  if (value == 0) {
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    return;
  }

  // Direção do par (mesma lógica do seu moveTank)
  if (value < 0) {         // sentido A
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
  } else {                 // sentido B
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
  }

  uint16_t mag = (uint16_t)abs(value);
  if (mag > _maxPwm) mag = _maxPwm;

  // ✅ aplica deadband antes do ganho (pra garantir partida)
  mag = applyDeadband(mag);

  // Ganho por motor + clamp
  uint16_t m1 = scaleAndClamp(mag, g1);
  uint16_t m2 = scaleAndClamp(mag, g2);

  analogWrite(pwm1, m1);
  analogWrite(pwm2, m2);
}

void Tank4Simple::moveTank(int left_value, int right_value, bool correctionFlag) {
  if (correctionFlag && _correctionFn) {
    left_value  = _correctionFn(left_value);
    right_value = _correctionFn(right_value);
  }

  left_value  = clamp_int(left_value,  -(int)_maxPwm, (int)_maxPwm);
  right_value = clamp_int(right_value, -(int)_maxPwm, (int)_maxPwm);

  applySide(left_value,  _in00a, _in00b, _pwm1, _pwm2, _g1, _g2);
  applySide(right_value, _in01a, _in01b, _pwm3, _pwm4, _g3, _g4);
}

void Tank4Simple::stop(bool brake) {
  analogWrite(_pwm1, 0);
  analogWrite(_pwm2, 0);
  analogWrite(_pwm3, 0);
  analogWrite(_pwm4, 0);

  if (brake) {
    digitalWrite(_in00a, HIGH); digitalWrite(_in00b, HIGH);
    digitalWrite(_in01a, HIGH); digitalWrite(_in01b, HIGH);
  } else {
    digitalWrite(_in00a, LOW);  digitalWrite(_in00b, LOW);
    digitalWrite(_in01a, LOW);  digitalWrite(_in01b, LOW);
  }
}
