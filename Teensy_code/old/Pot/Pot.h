#pragma once 
#include <Arduino.h>
#include "config.h"

class pot {
  public:
    pot(uint8_t pin, uint8_t filterSize = DEFAULT_FILTER);
    void begin();
    uint16_t read();
    uint32_t readFiltered();

    void updateFastFilter(uint16_t sample);
    uint16_t readFastFiltered() const;

  private:
    uint8_t _pin;
    uint8_t _filterSize;

    uint16_t _fastFiltered = 0;
    bool _fastInit  = false;
    
};


