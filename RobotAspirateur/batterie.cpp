#include <Arduino.h>
#include "batterie.h"
#include "config.h"
#include "debug.h"

void batterieInit() {
  pinMode(PIN_BATTERIE, INPUT);
  debugLog("[BATTERIE] Init OK");
}

float batterieLireTension() {
  if (MODE_SIMULATION) {
    // Descente douce simulée entre BATTERIE_TENSION_MAX et BATTERIE_TENSION_MIN.
    float phase = (millis() % 60000UL) / 60000.0f;
    return BATTERIE_TENSION_MAX - (BATTERIE_TENSION_MAX - BATTERIE_TENSION_MIN) * phase;
  }

  int valeurBrute = analogRead(PIN_BATTERIE);
  float tension = (valeurBrute / 1023.0f) * BATTERIE_TENSION_MAX;
  return tension;
}

bool batterieEstFaible() {
  return batterieLireTension() <= BATTERIE_TENSION_MIN;
}
