#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "capteurs.h"
#include "carte.h"

enum class ActionNavigation {
  Avancer,
  TournerGauche,
  TournerDroite,
  Reculer,
  ArretSecurite
};

// --- Navigation réactive (Modes Découverte) ---
void navigationInit();
void navigationReset(); // Remet la machine à états à zéro
ActionNavigation navigationChoisirAction(const EtatCapteurs& capteurs, bool batterieFaible);

// --- Navigation boustrophédon (Mode Nettoyage) ---
void navigationInitNettoyage(); // Génère le parcours en zigzag depuis la carte
ActionNavigation navigationChoisirActionNettoyage(PositionRobot pos,
                                                   const EtatCapteurs& capteurs,
                                                   bool batterieFaible);
bool navigationNettoyageFini(); // Vrai si tous les waypoints ont été visités

#endif
