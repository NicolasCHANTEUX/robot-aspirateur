#include <Arduino.h>
#include "navigation.h"
#include "debug.h"
#include "config.h"

// Variables pour mémoriser l'état en cours (Machine à États)
ActionNavigation etatCourant = ActionNavigation::Avancer;
unsigned long tempsDebutManoeuvre = 0;

void navigationInit() {
  debugLog("[NAVIGATION] Init OK - Machine à États activée");
}

ActionNavigation navigationChoisirAction(const EtatCapteurs& capteurs, bool batterieFaible) {
  if (batterieFaible) {
    return ActionNavigation::ArretSecurite;
  }

  unsigned long tempsActuel = millis();

  // 1. Est-ce qu'on est DÉJÀ en train de faire une manœuvre prioritaire ?
  if (etatCourant == ActionNavigation::Reculer) {
    // Séquence Anti-Vide : Reculer 300ms, PUIS Tourner 600ms
    unsigned long duree = tempsActuel - tempsDebutManoeuvre;
    if (duree < 300) {
      return ActionNavigation::Reculer;
    } else if (duree < 300 + DUREE_ROTATION_MS) {
      return ActionNavigation::TournerDroite; // On force la rotation après le recul !
    }
  } 
  else if (etatCourant == ActionNavigation::TournerGauche || 
           etatCourant == ActionNavigation::TournerDroite) {
    // Si la manœuvre dure depuis moins de 600ms, on la continue !
    if (tempsActuel - tempsDebutManoeuvre < DUREE_ROTATION_MS) {
      return etatCourant;
    }
  }

  // 2. Sinon, on lit les capteurs pour prendre une nouvelle décision
  if (capteurs.videDetecte) {
    etatCourant = ActionNavigation::Reculer;
    tempsDebutManoeuvre = tempsActuel; // On lance le chrono !
    return etatCourant;
  }

  if (capteurs.obstacleDevant) {
    etatCourant = ((tempsActuel / 1000UL) % 2UL == 0UL) ? ActionNavigation::TournerGauche : ActionNavigation::TournerDroite;
    tempsDebutManoeuvre = tempsActuel; // On lance le chrono !
    return etatCourant;
  }

  // Si tout va bien, on avance
  etatCourant = ActionNavigation::Avancer;
  return etatCourant;
}
