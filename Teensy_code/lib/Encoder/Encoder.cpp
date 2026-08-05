#include "Encoder.h"

Encoder* Encoder::_instance0 = nullptr;
Encoder* Encoder::_instance1 = nullptr;

Encoder::Encoder(uint8_t pinA, uint8_t pinB) : _pinA(pinA), _pinB(pinB), _ticks(0) {}

void Encoder::begin(bool pullup) {
  pinMode(_pinA, pullup ? INPUT_PULLUP : INPUT);
  pinMode(_pinB, pullup ? INPUT_PULLUP : INPUT);

  if (_instance0 == nullptr) {
    _instance0 = this;
    attachInterrupt(digitalPinToInterrupt(_pinA), Encoder::isr0, CHANGE);
  }
  else if (_instance1 == nullptr) {
    _instance1 = this;
    attachInterrupt(digitalPinToInterrupt(_pinA), Encoder::isr1, CHANGE);
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
  int Lstate = digitalRead(_pinA);
  if (Lstate == LOW && _lastStateA == HIGH) {
    val = digitalRead(_pinB);
    if (val == LOW && direcao) {
      direcao = false;
    } else if (val == HIGH && !direcao) {
      direcao = true;
    }
  }
  _lastStateA = Lstate;
  
  if (digitalRead(_pinB) == LOW) {
    _ticks++;  // Sentido horário / Frente
  } else {
    _ticks--;  // Sentido anti-horário / Trás
  }
}

void Encoder::isr0() {
  if (_instance0) _instance0->handleISR();
}

void Encoder::isr1() {
  if (_instance1) _instance1->handleISR();
}