#pragma once
#include <Arduino.h>

class Encoder {
public:
  explicit Encoder(uint8_t pinA, uint8_t pinB);

  void begin(bool pullup = true);

  void reset();
  int32_t read() const;
  int32_t readAndReset();

private:
  uint8_t _pinA;
  uint8_t _pinB;
  volatile int32_t _ticks;

  void handleISR();

  static void isr0();
  static void isr1();

  static Encoder* _instance0;
  static Encoder* _instance1;
};