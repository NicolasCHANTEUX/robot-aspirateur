#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "carte.h"

void communicationInit();
void communicationEnvoyerMiseAJour(PositionRobot pos, float distanceObstacle);
void communicationCleanupClients();

#endif
