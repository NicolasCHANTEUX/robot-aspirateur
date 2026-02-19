#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "carte.h"

enum class ActionNavigation {
  Avancer,
  TournerGauche,
  TournerDroite,
  Reculer,
  ArretSecurite
};

void navigationInit();
ActionNavigation navigationChoisirAction(const PositionRobot& pos, bool batterieFaible, float distanceObstacleCm);

#endif
