#include "BNO055_FranRobots.h"

BNO055_FranRobots::BNO055_FranRobots(uint8_t address)
: _address(address), _bno(55, address), _yawOffset(0.0f) {}

bool BNO055_FranRobots::begin(TwoWire& wire, uint32_t i2cClock) {
  wire.begin();
  wire.setClock(i2cClock);

  if (!_bno.begin())
    return false;

  _bno.setExtCrystalUse(true);
  _yawOffset = 0.0f;
  return true;
}

float BNO055_FranRobots::getYaw360() {
  imu::Vector<3> euler = _bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  float yaw = euler.x();
  yaw = wrap360(yaw - _yawOffset);
  return yaw;
}

float BNO055_FranRobots::getYaw() {
  float yaw = getYaw360();
  return (yaw > 180.0f) ? (yaw - 360.0f) : yaw;
}

void BNO055_FranRobots::zeroYaw() {
  imu::Vector<3> euler = _bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  _yawOffset = wrap360(euler.x());
}

float BNO055_FranRobots::wrap180(float angle) {
  while (angle > 180.0f) angle -= 360.0f;
  while (angle < -180.0f) angle += 360.0f;
  return angle;
}

float BNO055_FranRobots::wrap360(float angle) {
  while (angle >= 360.0f) angle -= 360.0f;
  while (angle < 0.0f) angle += 360.0f;
  return angle;
}