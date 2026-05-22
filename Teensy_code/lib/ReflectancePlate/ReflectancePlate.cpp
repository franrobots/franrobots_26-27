#include "ReflectancePlate.h"
#include <math.h>

ReflectancePlate::ReflectancePlate(const uint8_t* sensorPins, uint8_t sensorCount)
  : _pins(sensorPins), _count(sensorCount) {
  if (_count > 8) _count = 8;
  for (uint8_t i = 0; i < _count; i++) {
    _values[i] = 0;
    _ema[i] = 0.0f;
    _emaInit[i] = false;
  }
}

void ReflectancePlate::begin(uint8_t adcBits) {
  analogReadResolution(adcBits);
  for (uint8_t i = 0; i < _count; i++) {
    pinMode(_pins[i], INPUT);
  }
}

void ReflectancePlate::setEmaAlpha(float alpha) {
  if (alpha < 0.01f) alpha = 0.01f;
  if (alpha > 1.0f) alpha = 1.0f;
  _emaAlpha = alpha;
}

void ReflectancePlate::read(uint8_t numReadings, uint16_t interSampleDelayUs) {
  if (numReadings == 0) numReadings = 1;

  for (uint8_t n = 0; n < numReadings; n++) {
    for (uint8_t i = 0; i < _count; i++) {
      const uint16_t sample = (uint16_t)analogRead(_pins[i]);

      if (!_emaInit[i]) {
        _ema[i] = (float)sample;
        _emaInit[i] = true;
      } else {
        _ema[i] += _emaAlpha * ((float)sample - _ema[i]);
      }

      _values[i] = (uint16_t)(_ema[i] + 0.5f);
    }

    if (interSampleDelayUs) delayMicroseconds(interSampleDelayUs);
  }
}

uint16_t ReflectancePlate::value(uint8_t idx) const {
  if (idx >= _count) return 0;
  return _values[idx];
}

const uint16_t* ReflectancePlate::values() const {
  return _values;
}

uint16_t ReflectancePlate::c9() const {
  if (_c9Idx >= _count) return 0;
  return _values[_c9Idx];
}

void ReflectancePlate::setDecisionChannels(uint8_t c9Idx, uint8_t auxIdx) {
  if (_count == 0) return;
  _c9Idx = (c9Idx < _count) ? c9Idx : 0;
  _auxIdx = (auxIdx < _count) ? auxIdx : (_count - 1);
}

void ReflectancePlate::setMatchMode(ReflectanceMatchMode mode) {
  _matchMode = mode;
}

void ReflectancePlate::setRatioThresholdScale(float scale) {
  if (scale < 1.0f) scale = 1.0f;
  _ratioScale = scale;
}

void ReflectancePlate::setRedThreshold(const ColorThreshold& t)    { _red = t; }
void ReflectancePlate::setBlueThreshold(const ColorThreshold& t)   { _blue = t; }
void ReflectancePlate::setSilverThreshold(const ColorThreshold& t) { _silver = t; }
void ReflectancePlate::setBlackThreshold(const ColorThreshold& t)  { _black = t; }

void ReflectancePlate::setTiltFn(float (*fn)()) {
  _tiltFn = fn;
}

float ReflectancePlate::getTilt() const {
  if (!_tiltFn) return 0.0f;
  return _tiltFn();
}

bool ReflectancePlate::match(const ColorThreshold& t) const {
  if (t.ch0_max == 0 && t.ch3_max == 0) return false;

  const uint16_t ch0 = (_c9Idx < _count) ? _values[_c9Idx] : 0;
  const uint16_t ch3 = (_auxIdx < _count) ? _values[_auxIdx] : 0;

  if (_matchMode == ReflectanceMatchMode::RATIO) {
    const float denom = (ch3 > 0) ? (float)ch3 : 1.0f;
    const float ratio = ((float)ch0 / denom) * _ratioScale;
    const uint16_t r = (uint16_t)(ratio + 0.5f);
    if (r < t.ch0_min || r > t.ch0_max) return false;
  } else {
    if (ch0 < t.ch0_min || ch0 > t.ch0_max) return false;
    if (ch3 < t.ch3_min || ch3 > t.ch3_max) return false;
  }

  if (t.maxAbsTiltDeg > 0.0f) {
    const float tilt = fabsf(getTilt());
    if (tilt > t.maxAbsTiltDeg) return false;
  }

  return true;
}

FloorColor ReflectancePlate::detect() const {
  if (match(_black))  return FloorColor::BLACK;
  if (match(_blue))   return FloorColor::BLUE;
  if (match(_red))    return FloorColor::RED;
  if (match(_silver)) return FloorColor::SILVER;
  return FloorColor::UNKNOWN;
}

const char* ReflectancePlate::toString(FloorColor c) const {
  switch (c) {
    case FloorColor::RED:    return "red";
    case FloorColor::BLUE:   return "blue";
    case FloorColor::SILVER: return "silver";
    case FloorColor::BLACK:  return "black";
    default:                 return "unknown";
  }
}
