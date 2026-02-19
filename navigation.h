#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "capteurs.h"

enum class ActionNavigation {
  Avancer,
  TournerGauche,
  TournerDroite,
  Reculer,
  ArretSecurite
};

void navigationInit();
ActionNavigation navigationChoisirAction(const EtatCapteurs& capteurs, bool batterieFaible);

#endif
