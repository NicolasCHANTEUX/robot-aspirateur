#include <Arduino.h>
#include "config.h"
#include "debug.h"
#include "moteurs.h"

namespace {
bool g_moteursActifs = false; // État global des moteurs

void ecrireMoteur(int pinAvant, int pinArriere, int valeurAvant, int valeurArriere) {
  analogWrite(pinAvant, constrain(valeurAvant, 0, 255));
  analogWrite(pinArriere, constrain(valeurArriere, 0, 255));
}
}

void moteursInit() {
  pinMode(PIN_MOTEUR_GAUCHE_AVANT, OUTPUT);
  pinMode(PIN_MOTEUR_GAUCHE_ARRIERE, OUTPUT);
  pinMode(PIN_MOTEUR_DROIT_AVANT, OUTPUT);
  pinMode(PIN_MOTEUR_DROIT_ARRIERE, OUTPUT);
  moteursStop();
  debugLog("[MOTEURS] Init OK");
}

void moteursAvancer(int vitesse) {
  g_moteursActifs = true;
  if (MODE_SIMULATION) {
    debugLog("[SIM][MOTEURS] Avancer");
    return;
  }
  ecrireMoteur(PIN_MOTEUR_GAUCHE_AVANT, PIN_MOTEUR_GAUCHE_ARRIERE, vitesse, 0);
  ecrireMoteur(PIN_MOTEUR_DROIT_AVANT, PIN_MOTEUR_DROIT_ARRIERE, vitesse, 0);
}

void moteursReculer(int vitesse) {
  g_moteursActifs = true;
  if (MODE_SIMULATION) {
    debugLog("[SIM][MOTEURS] Reculer");
    return;
  }
  ecrireMoteur(PIN_MOTEUR_GAUCHE_AVANT, PIN_MOTEUR_GAUCHE_ARRIERE, 0, vitesse);
  ecrireMoteur(PIN_MOTEUR_DROIT_AVANT, PIN_MOTEUR_DROIT_ARRIERE, 0, vitesse);
}

void moteursTournerGauche(int vitesse) {
  g_moteursActifs = true;
  if (MODE_SIMULATION) {
    debugLog("[SIM][MOTEURS] Tourner gauche");
    return;
  }
  ecrireMoteur(PIN_MOTEUR_GAUCHE_AVANT, PIN_MOTEUR_GAUCHE_ARRIERE, 0, vitesse);
  ecrireMoteur(PIN_MOTEUR_DROIT_AVANT, PIN_MOTEUR_DROIT_ARRIERE, vitesse, 0);
}

void moteursTournerDroite(int vitesse) {
  g_moteursActifs = true;
  if (MODE_SIMULATION) {
    debugLog("[SIM][MOTEURS] Tourner droite");
    return;
  }
  ecrireMoteur(PIN_MOTEUR_GAUCHE_AVANT, PIN_MOTEUR_GAUCHE_ARRIERE, vitesse, 0);
  ecrireMoteur(PIN_MOTEUR_DROIT_AVANT, PIN_MOTEUR_DROIT_ARRIERE, 0, vitesse);
}

void moteursStop() {
  if (!g_moteursActifs) return; // Si déjà arrêtés, on ignore !
  g_moteursActifs = false;
  if (MODE_SIMULATION) {
    debugLog("[SIM][MOTEURS] Stop");
    return;
  }
  ecrireMoteur(PIN_MOTEUR_GAUCHE_AVANT, PIN_MOTEUR_GAUCHE_ARRIERE, 0, 0);
  ecrireMoteur(PIN_MOTEUR_DROIT_AVANT, PIN_MOTEUR_DROIT_ARRIERE, 0, 0);
}
