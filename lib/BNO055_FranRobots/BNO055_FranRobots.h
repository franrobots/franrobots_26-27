#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

class BNO055_FranRobots {
public:
  explicit BNO055_FranRobots(uint8_t address = 0x29);

  bool begin(TwoWire& wire = Wire, uint32_t i2cClock = 400000);

  float getYaw();        // -180..+180
  float getYaw360();     // 0..360
  void zeroYaw();

private:
  uint8_t _address;
  Adafruit_BNO055 _bno;
  float _yawOffset;

  static float wrap180(float angle);
  static float wrap360(float angle);
};