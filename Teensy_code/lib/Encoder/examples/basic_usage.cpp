#include <Arduino.h>
#include "Encoder.h"
#include "Motor.h"
  

motor m1(Ain1_M2, Ain2_M2);
motor m2(Ain1_M3, Ain2_M3);
Encoder enc1(21, 22);
Encoder enc2(26, 20);


void setup() {
  Serial.begin(115200);
  enc1.begin(true);
  enc2.begin(true);
  m1.begin(12, 30000);
  m2.begin(12, 30000);
  m1.setDeadband(800);
  m2.setDeadband(800);
}

void printEncoder(){
  Serial.print(" 1=");
  Serial.print(enc1.read());
  Serial.print(" 2=");
  Serial.println(enc2.read());
}

void rotina(){
   m2.applyPWM(-3000);
   m1.applyPWM(-3000);
   printEncoder();
  delay(3000);
   m2.applyPWM(3000);
   m1.applyPWM(3000);
   printEncoder();
  delay(3000);
  m2.stopMotor(true);
  m1.stopMotor(true);
  printEncoder();
  delay(3000);
}



void loop() {
rotina();
printEncoder();
}
