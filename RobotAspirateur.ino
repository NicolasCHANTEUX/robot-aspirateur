#include <Arduino.h>
#include "aspiration.h"
#include "batterie.h"
#include "capteurs.h"
#include "config.h"
#include "debug.h"
#include "moteurs.h"
#include "navigation.h"

// -----------------------------------------------------------------------
// setup()
// -----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  debugLog("=== Démarrage Robot Aspirateur ===");

  moteursInit();
  aspirationInit();
  capteursInit();
  batterieInit();
  navigationInit();

  debugLog("=== Initialisation terminée ===");
}

// -----------------------------------------------------------------------
// loop() – boucle de contrôle continue
// -----------------------------------------------------------------------
void loop() {
  // 1. Lecture des capteurs
  EtatCapteurs etat        = capteursLire();
  bool         batterieFaible = batterieEstFaible();

  // 2. Prise de décision
  ActionNavigation action = navigationChoisirAction(etat, batterieFaible);

  // 3. Action sur les moteurs et l'aspiration
  switch (action) {
    case ActionNavigation::Avancer:
      moteursAvancer();
      aspirationDemarrer();
      break;
    case ActionNavigation::Reculer:
      moteursReculer();
      aspirationArreter();
      break;
    case ActionNavigation::TournerGauche:
      moteursTournerGauche();
      aspirationArreter();
      break;
    case ActionNavigation::TournerDroite:
      moteursTournerDroite();
      aspirationArreter();
      break;
    case ActionNavigation::ArretSecurite:
    default:
      moteursStop();
      aspirationArreter();
      break;
  }

  // 4. Vérification de la batterie
  if (batterieFaible) {
    debugLog("[MAIN] Batterie faible ! Arrêt de sécurité.");
  }

  if (DEBUG_ACTIF) {
    String log = "[LOOP] obstacle=" + String(etat.obstacleDevant)
               + " vide=" + String(etat.videDetecte)
               + " tension=" + String(batterieLireTension(), 2) + "V";
    debugLog(log);
  }

  delay(200);
}

