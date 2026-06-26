#include "CalibrationMenu.h"

void CalibrationMenu::Update(uint8_t buttonPin, LedStrip &ledStrip,VL53Mux12_FRAN &tof, ReflectancePlate &floorSensor)
{
  bool btnPressionado = (digitalRead(buttonPin) == LOW);
  uint32_t now = millis();
  if (btnPressionado && _State == ROUND)
  {
    if (now - _timeBtnPressed >= 8000)
    {
      _State = MENU_CLICKING;
    }
  }
  else if (!btnPressionado && _State == ROUND)
  {
    _timeBtnPressed = now;
  }

  switch (_State)
  {
  case ROUND:
    break;

  case MENU_CLICKING:

    if (now - _timeLastColorChange >= 500){
      _timeLastColorChange = now;
      if (_ColorMenuNow == 1){
        _ColorMenuNow = 2;
      }
      else{
        _ColorMenuNow = 1;
      }

      ledStrip.clear();
      if (_ColorMenuNow == 1){
        ledStrip.setColor(LedSide::ALL, LedColor::BLUE);
      }
      else{
        ledStrip.setColor(LedSide::ALL, LedColor::RED);
      }
      ledStrip.update();
    }

    if (!btnPressionado){
      _State = MENU_CONFIRMED;
      _timetoConfirm = now;
    }
    break;

  case MENU_CONFIRMED:
    if (now - _timetoConfirm >= 8000){
      ledStrip.clear();
      ledStrip.update();
      _State = ROUND;
    }
    if (btnPressionado){
      if (_ColorMenuNow == 1){
        ToFCalibration::run(tof, buttonPin, ledStrip);
      }
      else{
        ColorCalibration::run(floorSensor, buttonPin, &ledStrip, 0, 3);
      }
      ledStrip.clear();
      ledStrip.update();
      _State = ROUND;
    }
    break;
  }
}