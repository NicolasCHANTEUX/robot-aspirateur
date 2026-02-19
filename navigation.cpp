#include <Arduino.h>
#include "navigation.h"
#include "debug.h"

void navigationInit() {
  debugLog("[NAVIGATION] Init OK");
}

ActionNavigation navigationChoisirAction(const EtatCapteurs& capteurs, bool batterieFaible) {
  if (batterieFaible) {
    return ActionNavigation::ArretSecurite;
  }

  if (capteurs.videDetecte) {
    return ActionNavigation::Reculer;
  }

  if (capteurs.obstacleDevant) {
    if ((millis() / 1000UL) % 2UL == 0UL) {
      return ActionNavigation::TournerGauche;
    }
    return ActionNavigation::TournerDroite;
  }

  return ActionNavigation::Avancer;
}
