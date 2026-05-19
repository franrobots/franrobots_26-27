#pragma once
#include "Arduino.h"
#include "config.h"

class led{
    public:
    led(uint8_t pin);
    void begin();
    void on();
    void off();
    void pwmApply(uint16_t value);
    void blick(uint16_t time);
    bool getState() const;

    private:
    uint8_t _pin;
    uint8_t _channel;
    uint16_t _resolution;
    uint16_t _frequency;
    uint32_t _lasttime;
    bool _state;

};