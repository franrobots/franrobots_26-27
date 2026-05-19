#include "Led.h"
#include <Arduino.h>

led::led(uint8_t pin)
    :_pin(pin),
    _channel(LED_CHANNEL),
    _resolution(LED_RESOLUTION),
    _frequency(FREQUENCY),
    _state(false),
    _lasttime(0)
{}

void led::begin(){
    pinMode(_pin,OUTPUT);
    analogWriteResolution(_resolution);
    analogWriteFrequency(_pin,_frequency);
    _lasttime = millis();
}

void led::on(){
    analogWrite(_pin, LED_MAX_VALUE);
    _state = true;
}

void led::off(){
    analogWrite(_pin, 0);
    _state = false;
}

void led::pwmApply(uint16_t value){
    (value > LED_MAX_VALUE) ? value = LED_MAX_VALUE : value;
    analogWrite(_pin, value);
    _state = (value > 0) ? true : false;
}

void led::blick(uint16_t time){
    if(millis() - _lasttime >= time){
        _lasttime = millis();
        _state ? off() : on();
    }
}

bool led::getState()const{
    return _state;
}