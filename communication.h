#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "carte.h"

void communicationInit();
void communicationEnvoyerMiseAJour(PositionRobot pos, float distanceObstacle, float niveauBatterie);
void communicationCleanupClients();
bool communicationRobotEnMarche(); // Retourne true si le robot doit fonctionner

#endif
