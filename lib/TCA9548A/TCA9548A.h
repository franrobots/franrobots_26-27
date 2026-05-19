#pragma once
#include <Arduino.h>
#include <Wire.h>

class TCA9548A {
public:
  explicit TCA9548A(uint8_t addr) : _addr(addr) {}

  bool begin(TwoWire& wire = Wire) {
    _wire = &wire;
    _wire->begin();
    return disableAll();
  }

  inline bool select(uint8_t ch) {
    if (!_wire || ch > 7) return false;
    disableAll();
    delayMicroseconds(100);
    _wire->beginTransmission(_addr);
    _wire->write(1u << ch);
    const bool ok = (_wire->endTransmission() == 0);
    delayMicroseconds(100);
    return ok;
  }

  inline bool disableAll() {
    if (!_wire) return false;
    _wire->beginTransmission(_addr);
    _wire->write(0x00);
    return (_wire->endTransmission() == 0);
  }

  uint8_t addr() const { return _addr; }

private:
  uint8_t _addr;
  TwoWire* _wire = nullptr;
};
