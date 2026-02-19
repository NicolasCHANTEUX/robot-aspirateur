#include <Arduino.h>
#include "odometrie.h"
#include "config.h"

volatile long g_ticsGauche = 0;
volatile long g_ticsDroite = 0;

void IRAM_ATTR isrEncodeurGauche() {
  // Lecture de la phase B pour déduire le sens sur le front montant de A.
  if (digitalRead(PIN_ENCODEUR_GAUCHE_B) == HIGH) {
    g_ticsGauche++;
  } else {
    g_ticsGauche--;
  }
}

void IRAM_ATTR isrEncodeurDroite() {
  if (digitalRead(PIN_ENCODEUR_DROITE_B) == HIGH) {
    g_ticsDroite++;
  } else {
    g_ticsDroite--;
  }
}

void odometrieInit() {
  pinMode(PIN_ENCODEUR_GAUCHE, INPUT_PULLUP);
  pinMode(PIN_ENCODEUR_GAUCHE_B, INPUT_PULLUP);
  pinMode(PIN_ENCODEUR_DROITE, INPUT_PULLUP);
  pinMode(PIN_ENCODEUR_DROITE_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_ENCODEUR_GAUCHE), isrEncodeurGauche, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODEUR_DROITE), isrEncodeurDroite, RISING);
}

float odometrieDistanceDepuisDerniereLectureCm() {
  static long precedentG = 0;
  static long precedentD = 0;

  noInterrupts();
  long tg = g_ticsGauche;
  long td = g_ticsDroite;
  interrupts();

  long deltaG = tg - precedentG;
  long deltaD = td - precedentD;
  precedentG = tg;
  precedentD = td;

  float toursMoyens = (deltaG + deltaD) / 2.0f / TICS_PAR_TOUR;
  float perimetreCm = PI * DIAMETRE_ROUE_CM;

  if (MODE_SIMULATION && deltaG == 0 && deltaD == 0) {
    return 0.35f;
  }

  return toursMoyens * perimetreCm;
}

long odometrieLireTicsGauche() { return g_ticsGauche; }
long odometrieLireTicsDroite() { return g_ticsDroite; }
