#include <Arduino.h>
#include "capteurs.h"
#include "config.h"
#include "debug.h"

void capteursInit() {
  pinMode(PIN_CAPTEUR_OBSTACLE, INPUT_PULLUP);
  pinMode(PIN_CAPTEUR_VIDE, INPUT_PULLUP);
  debugLog("[CAPTEURS] Init OK");
}

EtatCapteurs capteursLire() {
  if (MODE_SIMULATION) {
    // Simulation simple pseudo-périodique pour valider la logique.
    unsigned long t = millis();
    EtatCapteurs etat;
    etat.obstacleDevant = ((t / 5000UL) % 3UL) == 1UL;
    etat.videDetecte = ((t / 7000UL) % 5UL) == 2UL;
    return etat;
  }

  EtatCapteurs etat;
  etat.obstacleDevant = digitalRead(PIN_CAPTEUR_OBSTACLE) == LOW;
  etat.videDetecte = digitalRead(PIN_CAPTEUR_VIDE) == LOW;
  return etat;
}
