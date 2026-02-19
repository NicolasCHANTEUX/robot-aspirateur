#include <Arduino.h>
#include "aspiration.h"
#include "batterie.h"
#include "capteurs.h"
#include "config.h"
#include "debug.h"
#include "moteurs.h"
#include "navigation.h"

void appliquerAction(ActionNavigation action) {
  switch (action) {
    case ActionNavigation::Avancer:
      moteursAvancer();
      aspirationDemarrer();
      break;

    case ActionNavigation::Reculer:
      moteursReculer();
      aspirationDemarrer();
      delay(300);
      moteursTournerDroite();
      delay(DUREE_ROTATION_MS);
      break;

    case ActionNavigation::TournerGauche:
      moteursTournerGauche();
      aspirationDemarrer();
      delay(DUREE_ROTATION_MS);
      break;

    case ActionNavigation::TournerDroite:
      moteursTournerDroite();
      aspirationDemarrer();
      delay(DUREE_ROTATION_MS);
      break;

    case ActionNavigation::ArretSecurite:
      moteursStop();
      aspirationArreter();
      break;
  }
}

void setup() {
  Serial.begin(9600);
  debugLog("=== Démarrage Robot Aspirateur ===");

  moteursInit();
  aspirationInit();
  capteursInit();
  batterieInit();
  navigationInit();
}

void loop() {
  EtatCapteurs etat = capteursLire();
  bool batterieFaible = batterieEstFaible();

  ActionNavigation action = navigationChoisirAction(etat, batterieFaible);
  appliquerAction(action);

  if (DEBUG_ACTIF) {
    String log = "[LOOP] obstacle=" + String(etat.obstacleDevant)
               + " vide=" + String(etat.videDetecte)
               + " tension=" + String(batterieLireTension(), 2)
               + "V faible=" + String(batterieFaible);
    debugLog(log);
  }

  delay(200);
}
