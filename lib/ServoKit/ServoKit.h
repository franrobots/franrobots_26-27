#pragma once
#include <Arduino.h>
#include <Servo.h>

enum class ServoDropSide : uint8_t {
  LEFT,
  RIGHT
};

class ServoKit {
public:
  bool begin(uint8_t pin, uint16_t minUs = 1000, uint16_t maxUs = 2000);

  void setAngles(uint8_t centerDeg, uint8_t leftDropDeg, uint8_t rightDropDeg);
  void setTiming(uint16_t moveDelayMs, uint16_t betweenKitsMs);
  void setVictimKitMap(uint8_t noKitType, uint8_t oneKitType, uint8_t twoKitType);

  uint8_t kitsForVictim(uint8_t victimType) const;
  bool shouldDrop(uint8_t victimType) const;

  void center();
  void dropKits(ServoDropSide side, uint8_t kits);
  void dropForVictim(ServoDropSide side, uint8_t victimType);

private:
  Servo _servo;
  bool _attached = false;

  uint8_t _centerDeg = 90;
  uint8_t _leftDropDeg = 35;
  uint8_t _rightDropDeg = 145;
  uint16_t _moveDelayMs = 250;
  uint16_t _betweenKitsMs = 220;

  uint8_t _noKitType = 0;
  uint8_t _oneKitType = 1;
  uint8_t _twoKitType = 2;

  void moveTo(uint8_t deg);
};
