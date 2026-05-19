#pragma once
#include <Arduino.h>
#include "motor.h"
/*
      FRENTE
   [FL]     [FR]

   [RL]     [RR]
*/ 
class Robot {
public:
  Robot(motor& mFL, motor& mRL, motor& mFR, motor& mRR);

  void begin(uint8_t resolutionBits = 12, uint32_t freqHZ = 20000);

  void move_tank(int16_t linear, int16_t turn);
  void setLeftRight(int16_t left, int16_t right);

  void stop(bool brake = true);


  void turnLeft(int16_t speed);
  void turnRight(int16_t speed);
  void axis_curve(int16_t speed, const char* axis);

  void invertFL(bool inv);
  void invertRL(bool inv);
  void invertFR(bool inv);
  void invertRR(bool inv);

private:
  motor* _mFL;
  motor* _mRL;
  motor* _mFR;
  motor* _mRR;

  bool _invFL = false;
  bool _invRL = false;
  bool _invFR = false;
  bool _invRR = false;

  inline int16_t inv(int16_t v, bool flag) const {
    return flag ? (int16_t)(-v) : v;
  }
};