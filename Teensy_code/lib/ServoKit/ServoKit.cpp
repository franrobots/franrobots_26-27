#include "ServoKit.h"

bool ServoKit::begin(uint8_t pin, uint16_t minUs, uint16_t maxUs) {
  _attached = _servo.attach(pin, minUs, maxUs);
  if (_attached) center();
  return _attached;
}

void ServoKit::setAngles(uint8_t centerDeg, uint8_t leftDropDeg, uint8_t rightDropDeg) {
  _centerDeg = centerDeg;
  _leftDropDeg = leftDropDeg;
  _rightDropDeg = rightDropDeg;
}

void ServoKit::setTiming(uint16_t moveDelayMs, uint16_t betweenKitsMs) {
  _moveDelayMs = moveDelayMs;
  _betweenKitsMs = betweenKitsMs;
}

void ServoKit::setVictimKitMap(uint8_t noKitType, uint8_t oneKitType, uint8_t twoKitType) {
  _noKitType = noKitType;
  _oneKitType = oneKitType;
  _twoKitType = twoKitType;
}

uint8_t ServoKit::kitsForVictim(uint8_t victimType) const {
  if (victimType == _oneKitType) return 1;
  if (victimType == _twoKitType) return 2;
  return 0;
}

bool ServoKit::shouldDrop(uint8_t victimType) const {
  return kitsForVictim(victimType) > 0;
}

void ServoKit::moveTo(uint8_t deg) {
  if (!_attached) return;
  _servo.write(deg);
  delay(_moveDelayMs);
}

void ServoKit::center() {
  moveTo(_centerDeg);
}

void ServoKit::dropKits(ServoDropSide side, uint8_t kits) {
  if (!_attached || kits == 0) return;

  const uint8_t dropDeg = (side == ServoDropSide::LEFT) ? _leftDropDeg : _rightDropDeg;
  for (uint8_t i = 0; i < kits; i++) {
    moveTo(dropDeg);
    moveTo(_centerDeg);
    if (i + 1 < kits) delay(_betweenKitsMs);
  }
}

void ServoKit::dropForVictim(ServoDropSide side, uint8_t victimType) {
  dropKits(side, kitsForVictim(victimType));
}

void ServoKit::moveForMs(ServoDropSide side, uint16_t ms) {
  if (!_attached) return;
  _servo.writeMicroseconds(ms);
}
