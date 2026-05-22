#include "Drive4_PairedDir_Teensy.h"

static inline int clamp_int(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

Drive4_PairedDir::Drive4_PairedDir(MotorPairDirTB6612& pairLeft, MotorPairDirTB6612& pairRight)
: _left(pairLeft), _right(pairRight),
  _correctionFn(nullptr),
  _g1(1.0f), _g2(1.0f), _g3(1.0f), _g4(1.0f),
  _pwmMaxCached(4095)
{}

void Drive4_PairedDir::begin(uint8_t pwmResolutionBits, uint32_t pwmFreqHz) {
  _left.begin(pwmResolutionBits, pwmFreqHz);
  _right.begin(pwmResolutionBits, pwmFreqHz);

  _pwmMaxCached = (uint16_t)((1u << pwmResolutionBits) - 1u);
}

void Drive4_PairedDir::setCorrectionFn(int (*fn)(int)) { _correctionFn = fn; }

void Drive4_PairedDir::setMotorGains(float g1, float g2, float g3, float g4) {
  // Segurança: evita ganho negativo/zero
  _g1 = (g1 <= 0.05f) ? 0.05f : g1;
  _g2 = (g2 <= 0.05f) ? 0.05f : g2;
  _g3 = (g3 <= 0.05f) ? 0.05f : g3;
  _g4 = (g4 <= 0.05f) ? 0.05f : g4;
}

void Drive4_PairedDir::setStopModeAll(MotorPairDirTB6612::StopMode mode) {
  _left.setStopMode(mode);
  _right.setStopMode(mode);
}

void Drive4_PairedDir::setDeadbandAll(uint16_t db) {
  _left.setDeadband(db);
  _right.setDeadband(db);
}

void Drive4_PairedDir::setMaxPwmAll(uint16_t maxPwm) {
  _left.setMaxPwm(maxPwm);
  _right.setMaxPwm(maxPwm);
  _pwmMaxCached = maxPwm;
}

uint16_t Drive4_PairedDir::scaleMag(int baseCmdAbs, float gain, uint16_t pwmMax) const {
  // baseCmdAbs normalmente já está em escala de PWM (ex: 0..4095)
  float v = (float)baseCmdAbs * gain;
  if (v < 0.0f) v = 0.0f;
  if (v > (float)pwmMax) v = (float)pwmMax;
  return (uint16_t)lroundf(v);
}

void Drive4_PairedDir::stopAll(MotorPairDirTB6612::StopMode mode) {
  _left.stop(mode);
  _right.stop(mode);
}

void Drive4_PairedDir::moveTank(int left_value, int right_value, bool correctionFlag) {
  if (correctionFlag && _correctionFn) {
    left_value  = _correctionFn(left_value);
    right_value = _correctionFn(right_value);
  }

  left_value  = clamp_int(left_value,  -(int)_pwmMaxCached, (int)_pwmMaxCached);
  right_value = clamp_int(right_value, -(int)_pwmMaxCached, (int)_pwmMaxCached);

  const int leftAbs  = abs(left_value);
  const int rightAbs = abs(right_value);

  // Par esquerdo (M1 e M2) com ganhos independentes
  uint16_t pwmM1 = scaleMag(leftAbs,  _g1, _pwmMaxCached);
  uint16_t pwmM2 = scaleMag(leftAbs,  _g2, _pwmMaxCached);
  _left.drive(left_value, pwmM1, pwmM2);

  // Par direito (M3 e M4) com ganhos independentes
  uint16_t pwmM3 = scaleMag(rightAbs, _g3, _pwmMaxCached);
  uint16_t pwmM4 = scaleMag(rightAbs, _g4, _pwmMaxCached);
  _right.drive(right_value, pwmM3, pwmM4);
}
