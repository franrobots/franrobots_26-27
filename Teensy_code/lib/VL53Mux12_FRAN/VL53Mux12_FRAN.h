#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

enum class ToFId : uint8_t {
  FL, FC, FR,
  LF, LC, LB,
  BL,     BR,
  RF, RC, RB,
  COUNT
};

struct ScanToF12 {
  uint16_t mm[(uint8_t)ToFId::COUNT];
  bool ok[(uint8_t)ToFId::COUNT];
  uint32_t age_ms[(uint8_t)ToFId::COUNT];
};

class VL53Mux12_FRAN {
public:
  VL53Mux12_FRAN(uint8_t addrA, uint8_t addrB);
  
  bool begin(TwoWire& wire = Wire, uint32_t i2cClock = 400000, uint16_t periodMs = 40);
  uint16_t readSensor(ToFId id);
  void readAll();
  bool update();
  void printAll(const uint16_t* mm) const;
  void printAll(const ScanToF12& scan) const;
  void snapshot(ScanToF12& out) const;
  void getSensorOk() const;

  void setOffset(ToFId id, int16_t off);
  void setEmaAlpha(float alpha);

  uint16_t getRaw(ToFId id) const;
  uint16_t getCal(ToFId id) const;
  uint16_t get(ToFId id) const;
  bool valid(ToFId id) const;
  bool sensorInitOk(ToFId id) const;

private:
  uint8_t _addrA, _addrB;
  TwoWire* _wire = nullptr;

  VL53L0X _sensor[(uint8_t)ToFId::COUNT];

  uint16_t _raw[(uint8_t)ToFId::COUNT];
  uint16_t _cal[(uint8_t)ToFId::COUNT];
  uint16_t _mm[(uint8_t)ToFId::COUNT];
  bool _ok[(uint8_t)ToFId::COUNT];
  uint32_t _last[(uint8_t)ToFId::COUNT];
  bool _sensorOk[(uint8_t)ToFId::COUNT];

  int16_t _offset[(uint8_t)ToFId::COUNT];

  float _ema[(uint8_t)ToFId::COUNT];
  bool _emaInit[(uint8_t)ToFId::COUNT];
  float _alpha = 0.25f;

  uint8_t _rr = 0;
  uint16_t _period = 40;

  void tcaDisable(uint8_t addr);
  void tcaDisableAll();
  void tcaSelect(uint8_t addr, uint8_t ch);
  bool readSensorByIndex(uint8_t i);
};
