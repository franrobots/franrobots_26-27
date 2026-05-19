#include <Arduino.h>
#include "OpenMVCamera.h"

OpenMVCamera cam(0x10);

void setup() {
  Serial.begin(115200);
  cam.begin(400000);
}

void loop() {
  CameraData d = cam.read(15, 0);
  Serial.print("victim=");
  Serial.print(d.vitima);
  Serial.print(" conf=");
  Serial.println(d.confianca);
  delay(80);
}
