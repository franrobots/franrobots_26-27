#include "Encoder.h"

Encoder* Encoder::_instance0 = nullptr;
Encoder* Encoder::_instance1 = nullptr;

Encoder::Encoder(uint8_t pin)
: _pin(pin), _ticks(0) {}

void Encoder::begin(bool pullup) {
  pinMode(_pin, pullup ? INPUT_PULLUP : INPUT);

  if (_instance0 == nullptr) {
    _instance0 = this;
    attachInterrupt(digitalPinToInterrupt(_pin), Encoder::isr0, RISING);
  }
  else if (_instance1 == nullptr) {
    _instance1 = this;
    attachInterrupt(digitalPinToInterrupt(_pin), Encoder::isr1, RISING);
  }
}

void Encoder::reset() {
  noInterrupts();
  _ticks = 0;
  interrupts();
}

int32_t Encoder::read() const {
  noInterrupts();
  int32_t v = _ticks;
  interrupts();
  return v;
}

int32_t Encoder::readAndReset() {
  noInterrupts();
  int32_t v = _ticks;
  _ticks = 0;
  interrupts();
  return v;
}

void Encoder::handleISR() {
  _ticks++;
}

void Encoder::isr0() {
  if (_instance0) _instance0->handleISR();
}

void Encoder::isr1() {
  if (_instance1) _instance1->handleISR();
}