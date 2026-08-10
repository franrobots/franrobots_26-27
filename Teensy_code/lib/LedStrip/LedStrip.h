#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

enum class LedColor : uint8_t {
  GREEN,
  YELLOW,
  RED,
  BLUE,
  WHITE,
  OFF
};

enum class LedSide : uint8_t {
  LEFT,
  RIGHT,
  ALL
};

class LedStrip {
public:
  LedStrip(uint8_t pin, uint16_t nLeds);

  void begin();
  void update(); // chama no loop

  void setBrightness(uint32_t intensity);
  void setColor(LedSide side, LedColor color);
  void clear();

  void blink(LedSide side, LedColor color, uint8_t repetitions, uint16_t intervalMs = 400);
  void setpixel(LedSide side, uint8_t index, LedColor color);

private:
  Adafruit_NeoPixel _strip;

  uint16_t _nLeds;

  // Blink state
  bool _blinking = false;
  uint8_t _blinkCount = 0;
  uint8_t _blinkTarget = 0;
  uint32_t _lastToggle = 0;
  uint16_t _interval = 400;
  LedSide _blinkSide;
  LedColor _blinkColor;
  bool _blinkOn = false;

  uint32_t colorToRGB(LedColor c);
  void applySide(LedSide side, uint32_t rgb);
};