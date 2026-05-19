#pragma once
#include <Arduino.h>

enum class SwitchEvent : uint8_t {
  NONE,
  LEFT,
  RIGHT,
  BOTH
};

class SwitchAvoidance {
public:
  SwitchAvoidance(uint8_t leftPin, uint8_t rightPin);

  void begin(bool pullup = true);
  SwitchEvent poll();

  bool leftPressed() const { return _leftStable; }
  bool rightPressed() const { return _rightStable; }

private:
  uint8_t _leftPin;
  uint8_t _rightPin;
  bool _pullup = true;

  bool _leftStable = false;
  bool _rightStable = false;
  bool _lastActive = false;

  bool readDebouncedPressed(uint8_t pin) const;
};
