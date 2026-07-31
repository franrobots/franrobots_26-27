#include "Robot.h"

Robot::Robot(motor& mFL, motor& mRL, motor& mFR, motor& mRR)
: _mFL(&mFL), _mRL(&mRL), _mFR(&mFR), _mRR(&mRR) {}

void Robot::begin(uint8_t resolutionBits, uint32_t freqHZ) {
  _mFL->begin(resolutionBits, freqHZ);
  _mRL->begin(resolutionBits, freqHZ);
  _mFR->begin(resolutionBits, freqHZ);
  _mRR->begin(resolutionBits, freqHZ);
}

void Robot::invertFL(bool invFlag) { _invFL = invFlag; }
void Robot::invertRL(bool invFlag) { _invRL = invFlag; }
void Robot::invertFR(bool invFlag) { _invFR = invFlag; }
void Robot::invertRR(bool invFlag) { _invRR = invFlag; }

void Robot::move_tank(int16_t linear, int16_t turn) {
  int32_t left  = (int32_t)linear + (int32_t)turn;
  int32_t right = (int32_t)linear - (int32_t)turn;

  setLeftRight((int16_t)left, (int16_t)right);
}

void Robot::setLeftRight(int16_t left, int16_t right) {
  int16_t fl = inv(left,  _invFL);
  int16_t rl = inv(left,  _invRL);
  int16_t fr = inv(right, _invFR);
  int16_t rr = inv(right, _invRR);

  _mFL->applyPWM(fl);
  _mRL->applyPWM(rl);
  _mFR->applyPWM(fr);
  _mRR->applyPWM(rr);
}

void Robot::turnLeft(int16_t speed) {
  move_tank(0, speed);
}

void Robot::turnRight(int16_t speed) {
  move_tank(0, -speed);
}

void Robot::stop(bool brake) {
  _mFL->stopMotor(brake);
  _mRL->stopMotor(brake);
  _mFR->stopMotor(brake);
  _mRR->stopMotor(brake);
}