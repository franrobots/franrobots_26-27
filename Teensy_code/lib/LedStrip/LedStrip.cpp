#include "LedStrip.h"

LedStrip::LedStrip(uint8_t pin, uint16_t nLeds)
: _strip(nLeds, pin, NEO_GRB + NEO_KHZ800),
  _nLeds(nLeds) {}

void LedStrip::begin() {
  _strip.begin();
  _strip.clear();
  _strip.show();
}

uint32_t LedStrip::colorToRGB(LedColor c) {
  switch (c) {
    case LedColor::GREEN:  return 0x00FF00;
    case LedColor::YELLOW: return 0xFFFF00;
    case LedColor::RED:    return 0xFF0000;
    case LedColor::BLUE:   return 0x0000FF;
    case LedColor::WHITE:  return 0xFFFFFF;
    default:               return 0x000000;
  }
}

void LedStrip::applySide(LedSide side, uint32_t rgb) {
  if (side == LedSide::ALL) {
    for (uint16_t i = 0; i < _nLeds; i++)
      _strip.setPixelColor(i, rgb);
  }
  else if (side == LedSide::LEFT) {
    _strip.setPixelColor(0, rgb);
    _strip.setPixelColor(1, rgb);
    _strip.setPixelColor(2, rgb);
  }
  else if (side == LedSide::RIGHT) {
    _strip.setPixelColor(10, rgb);
    _strip.setPixelColor(11, rgb);
    _strip.setPixelColor(12, rgb);
  }
}

void LedStrip::setBrightness(uint32_t intensity) {
  _strip.setBrightness(intensity);
  _strip.show();
}

void LedStrip::setColor(LedSide side, LedColor color) {
  clear();
  applySide(side, colorToRGB(color));
  _strip.show();
}

void LedStrip::clear() {
  for (uint16_t i = 0; i < _nLeds; i++)
    _strip.setPixelColor(i, 0);
  _strip.show();
}

void LedStrip::blink(LedSide side, LedColor color, uint8_t repetitions, uint16_t intervalMs) {
  _blinking = true;
  _blinkSide = side;
  _blinkColor = color;
  _blinkTarget = repetitions * 2; // on + off
  _blinkCount = 0;
  _interval = intervalMs;
  _lastToggle = millis();
  _blinkOn = false;
}

void LedStrip::setpixel(LedSide side, uint8_t index, LedColor color) {
  if (side == LedSide::ALL) {
    if (index < _nLeds) {
      _strip.setPixelColor(index, colorToRGB(color));
    }
  }
  else if (side == LedSide::LEFT && index < 2) {
    _strip.setPixelColor(index, colorToRGB(color));
  } 
  else if (side == LedSide::RIGHT && index < 2) {
    _strip.setPixelColor(index + 3, colorToRGB(color));
  }
  _strip.show(); 
}

void LedStrip::update() {
  if (!_blinking) return;

  if (millis() - _lastToggle >= _interval) {
    _lastToggle = millis();

    if (_blinkOn) {
      clear();
    } else {
      applySide(_blinkSide, colorToRGB(_blinkColor));
      _strip.show();
    }

    _blinkOn = !_blinkOn;
    _blinkCount++;

    if (_blinkCount >= _blinkTarget) {
      _blinking = false;
      clear();
    }
  }
}