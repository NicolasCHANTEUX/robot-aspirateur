#include <Arduino.h>
#include "aspiration.h"
#include "config.h"
#include "debug.h"

namespace {
bool g_aspirationActive = false;
}

void aspirationInit() {
  pinMode(PIN_TURBINE, OUTPUT);
  aspirationArreter();
  debugLog("[ASPIRATION] Init OK");
}

void aspirationDemarrer(int puissance) {
  g_aspirationActive = true;
  if (MODE_SIMULATION) {
    debugLog("[SIM][ASPIRATION] Démarrer");
    return;
  }
  analogWrite(PIN_TURBINE, constrain(puissance, 0, 255));
}

void aspirationArreter() {
  if (!g_aspirationActive) return; // Si déjà arrêté, on ignore !
  g_aspirationActive = false;
  if (MODE_SIMULATION) {
    debugLog("[SIM][ASPIRATION] Arrêter");
    return;
  }
  analogWrite(PIN_TURBINE, 0);
}

bool aspirationEstActive() {
  return g_aspirationActive;
}
