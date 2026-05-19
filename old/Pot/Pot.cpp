#include "Pot.h"

pot::pot(uint8_t pin, uint8_t filterSize)
:_pin(pin),
 _filterSize(filterSize){}

void pot::begin(){
    pinMode(_pin, INPUT);
    analogReadResolution(LED_RESOLUTION);
}

uint16_t pot::read() {
  return analogRead(_pin);
}

uint32_t pot::readFiltered() {

  uint32_t sum = 0;
  for (uint8_t i = 0; i < _filterSize; i++) {
    sum += read();
    delay(10); // Small delay between readings
  }
  return sum / _filterSize;
}

void pot::updateFastFilter(uint16_t sample) {
  if (!_fastInit) {
    _fastFiltered = sample;
    _fastInit = true;
    return;
  }

  // EMA por shift (rápido): alpha = 1/8
  // quanto maior o divisor (ex: 16), mais suave e mais lento
  constexpr uint8_t SHIFT = 3; // 1/8
  _fastFiltered = _fastFiltered + ((int32_t)sample - _fastFiltered) / (1 << SHIFT);
}

uint16_t pot::readFastFiltered() const {
  return _fastFiltered;
}