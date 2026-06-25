#include "CalibrationMenu.h"

void CalibrationMenu::Update(uint8_t buttonPin, LedStrip &ledStrip)
{
  bool btnPressionado = (digitalRead(buttonPin) == LOW);
  uint32_t now = millis();
  if (btnPressionado && _State == ROUND) {
    if (now - _timeBtnPressed >= 8000)
    {
      _State = MENU_CLICKING;
    }
  }
  else if (!btnPressionado && _State == ROUND){
    _timeBtnPressed = now;
  }

  if (!btnPressionado && _State != ROUND){
    if (now - _timeFreeBtn >= 8000)
    {
      _State = ROUND; 
      ledStrip.clear();
      return;
    }
  }
  else if (btnPressionado){
    _timeFreeBtn = now; 
  }
}