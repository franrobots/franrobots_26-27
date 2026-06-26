#include <Arduino.h>
#include "ToFCalibration.h"
#include "ColorCalibration.h"
#include "LedStrip.h"

enum MenuOptions
{
  ROUND,
  WAITING_8S,
  MENU_CLICKING,
  MENU_CONFIRMED,
  CALIBRATING_TOF,
  CALIBRATING_COLOR

};

class CalibrationMenu
{
public:
    void Update(uint8_t buttonPin, LedStrip& ledStrip,VL53Mux12_FRAN &tof, ReflectancePlate &floorSensor);

private:
    uint32_t _timeBtnPressed = 0;
    uint32_t _timeFreeBtn = millis();
    uint32_t _timeLastColorChange = 0;
    uint8_t _ColorMenuNow = 1; 
    uint8_t _timetoConfirm = 0;
    MenuOptions _State = ROUND;
};


