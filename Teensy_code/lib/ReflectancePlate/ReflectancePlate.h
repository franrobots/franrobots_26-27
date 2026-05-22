#pragma once
#include <Arduino.h>

enum class FloorColor : uint8_t {
  UNKNOWN,
  RED,
  BLUE,
  SILVER,
  BLACK
};

enum class ReflectanceMatchMode : uint8_t {
  ABSOLUTE,
  RATIO
};

struct ColorThreshold {
  // Uses channels idx0 and idx3 as primary features.
  uint16_t ch0_min;
  uint16_t ch0_max;
  uint16_t ch3_min;
  uint16_t ch3_max;

  // Optional tilt filter in degrees. <= 0 disables.
  float maxAbsTiltDeg;
};

class ReflectancePlate {
public:
  ReflectancePlate(const uint8_t* sensorPins, uint8_t sensorCount);

  void begin(uint8_t adcBits = 12);

  // EMA-based read (no arithmetic mean filter).
  // numReadings = number of sequential EMA updates in this call.
  void read(uint8_t numReadings = 1, uint16_t interSampleDelayUs = 200);
  void setEmaAlpha(float alpha);

  uint16_t value(uint8_t idx) const;
  const uint16_t* values() const;
  uint16_t c9() const;
  void setDecisionChannels(uint8_t c9Idx, uint8_t auxIdx = 3);
  void setMatchMode(ReflectanceMatchMode mode);
  void setRatioThresholdScale(float scale);

  void setRedThreshold(const ColorThreshold& t);
  void setBlueThreshold(const ColorThreshold& t);
  void setSilverThreshold(const ColorThreshold& t);
  void setBlackThreshold(const ColorThreshold& t);

  void setTiltFn(float (*fn)());

  FloorColor detect() const;
  const char* toString(FloorColor c) const;

private:
  const uint8_t* _pins;
  uint8_t _count;

  uint16_t _values[8];
  float _ema[8];
  bool _emaInit[8];
  float _emaAlpha = 0.35f;
  uint8_t _c9Idx = 0;
  uint8_t _auxIdx = 3;
  ReflectanceMatchMode _matchMode = ReflectanceMatchMode::ABSOLUTE;
  float _ratioScale = 1000.0f;

  float (*_tiltFn)() = nullptr;

  ColorThreshold _red   {0,0,0,0,0};
  ColorThreshold _blue  {0,0,0,0,0};
  ColorThreshold _silver{0,0,0,0,0};
  ColorThreshold _black {0,0,0,0,0};

  bool match(const ColorThreshold& t) const;
  float getTilt() const;
};
