#include <Arduino.h>
#include "ultrasons.h"
#include "config.h"

void ultrasonsInit() {
  pinMode(PIN_US_TRIG_AVANT, OUTPUT);
  pinMode(PIN_US_ECHO_AVANT, INPUT);
}

float ultrasonsDistanceAvantCm() {
  if (MODE_SIMULATION) {
    if (((millis() / 4000UL) % 2UL) == 0UL) {
      return 120.0f;
    }
    return 30.0f;
  }

  digitalWrite(PIN_US_TRIG_AVANT, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_US_TRIG_AVANT, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG_AVANT, LOW);

  unsigned long duree = pulseIn(PIN_US_ECHO_AVANT, HIGH, 30000UL);
  if (duree == 0) {
    return 400.0f;
  }
  return (duree * 0.0343f) / 2.0f;
}
