#include "SwitchAvoidance.h"

SwitchAvoidance::SwitchAvoidance(uint8_t leftPin, uint8_t rightPin)
  : _leftPin(leftPin), _rightPin(rightPin) {}

void SwitchAvoidance::begin(bool pullup) {
  _pullup = pullup;
  pinMode(_leftPin, _pullup ? INPUT_PULLUP : INPUT);
  pinMode(_rightPin, _pullup ? INPUT_PULLUP : INPUT);
}

bool SwitchAvoidance::readDebouncedPressed(uint8_t pin) const {
  uint8_t pressedHits = 0;
  for (uint8_t i = 0; i < 3; i++) {
    const int v = digitalRead(pin);
    const bool pressed = _pullup ? (v == LOW) : (v == HIGH);
    if (pressed) pressedHits++;
    delay(2);
  }
  return pressedHits >= 2;
}

SwitchEvent SwitchAvoidance::poll() {
  _leftStable = readDebouncedPressed(_leftPin);
  _rightStable = readDebouncedPressed(_rightPin);

  const bool active = _leftStable || _rightStable;
  SwitchEvent ev = SwitchEvent::NONE;

  if (active && !_lastActive) {
    if (_leftStable && _rightStable) ev = SwitchEvent::BOTH;
    else if (_leftStable) ev = SwitchEvent::LEFT;
    else if (_rightStable) ev = SwitchEvent::RIGHT;
  }

  _lastActive = active;
  return ev;
}
