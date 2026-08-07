#include <Arduino.h>
#include "config.h"
#include "ReflectancePlate.h"
#include "LedStrip.h"
const uint8_t pins[] = {C9_PIN, LDR_RED,LDR_GREEN, LDR_BLUE};
// const uint8_t pins[] = {A2, A1, A3, A0}; // C9, R, G, B (exemplo)
ReflectancePlate plate(pins, 4);
LedStrip led_MIDDLE(LED_MIDDLE, 7);
void led_show(FloorColor colornum);
void setup() {
  Serial.begin(115200);
  plate.begin(ADC_RESOLUTION);
  plate.setEmaAlpha(0.30f);
  plate.setDecisionChannels(0, 2);
  plate.setMatchMode(ReflectanceMatchMode::RATIO);
  plate.setRatioThresholdScale(1000.0f);
  led_MIDDLE.begin();
  led_MIDDLE.clear();
  led_MIDDLE.setColor(LedSide::ALL, LedColor::WHITE);
  plate.setBlueThreshold({20, 80, 0, 0, 0.0f});
  plate.setSilverThreshold({90, 130, 0, 0, 0.0f});
  plate.setRedThreshold({135, 160, 0, 0, 0.0f});
  plate.setBlackThreshold({230, 400, 0, 0, 0.0f});
}

void loop() {
led_MIDDLE.update();
FloorColor color = plate.detect();
plate.read(1, 200);
uint16_t c9 = plate.value(0);
uint16_t g = plate.value(2);

float ratio = 0;

if(g>0){
  ratio = ((float)c9 / (float)g) * 1000.0f;
}
Serial.print("C9=");
Serial.print(c9);

Serial.print(" G=");
Serial.print(g);

Serial.print(" ratio=");
Serial.print(ratio);

Serial.print(" color=");
Serial.println(plate.toString(color));
led_show(color);
}

void led_show(FloorColor colornum){
  switch (colornum)
  {
    case FloorColor::UNKNOWN://unknown
    led_MIDDLE.clear();
    led_MIDDLE.setColor(LedSide::ALL, LedColor::OFF);
    break;
    case FloorColor::RED:// red
    led_MIDDLE.clear();
    led_MIDDLE.setColor(LedSide::ALL, LedColor::RED);
    break;
    case FloorColor::BLUE: //blue
    led_MIDDLE.clear();
    led_MIDDLE.setColor(LedSide::ALL, LedColor::BLUE);
    break;
    case FloorColor::SILVER:// silver
    led_MIDDLE.clear();
    led_MIDDLE.setColor(LedSide::ALL, LedColor::YELLOW);
    break;
    case FloorColor::BLACK: // black
    led_MIDDLE.clear();
    led_MIDDLE.setColor(LedSide::ALL, LedColor::GREEN);
     break;
 }

}



