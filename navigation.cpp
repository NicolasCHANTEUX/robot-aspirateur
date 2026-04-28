#include <Arduino.h>
#include "navigation.h"
#include "debug.h"
#include "config.h"

// ======================================================================
// État interne – navigation réactive
// ======================================================================
namespace {
ActionNavigation etatCourant = ActionNavigation::Avancer;
unsigned long tempsDebutManoeuvre = 0;
} // namespace

// ======================================================================
// Navigation réactive
// ======================================================================

void navigationInit() {
  debugLog("[NAVIGATION] Init OK - Machine à États activée");
}

void navigationReset() {
  etatCourant = ActionNavigation::Avancer;
  tempsDebutManoeuvre = 0;
}

ActionNavigation navigationChoisirAction(const EtatCapteurs& capteurs, bool batterieFaible) {
  if (batterieFaible) {
    return ActionNavigation::ArretSecurite;
  }

  unsigned long tempsActuel = millis();

  // 1. Continuer la manœuvre en cours ?
  if (etatCourant == ActionNavigation::Reculer) {
    unsigned long duree = tempsActuel - tempsDebutManoeuvre;
    if (duree < 300) {
      return ActionNavigation::Reculer;
    } else if (duree < 300 + DUREE_ROTATION_MS) {
      return ActionNavigation::TournerDroite;
    }
  } else if (etatCourant == ActionNavigation::TournerGauche ||
             etatCourant == ActionNavigation::TournerDroite) {
    if (tempsActuel - tempsDebutManoeuvre < DUREE_ROTATION_MS) {
      return etatCourant;
    }
  }

  // 2. Lire les capteurs
  if (capteurs.videDetecte) {
    etatCourant = ActionNavigation::Reculer;
    tempsDebutManoeuvre = tempsActuel;
    return etatCourant;
  }

  if (capteurs.obstacleDevant) {
    etatCourant = ((tempsActuel / 1000UL) % 2UL == 0UL)
                  ? ActionNavigation::TournerGauche
                  : ActionNavigation::TournerDroite;
    tempsDebutManoeuvre = tempsActuel;
    return etatCourant;
  }

  etatCourant = ActionNavigation::Avancer;
  return etatCourant;
}

