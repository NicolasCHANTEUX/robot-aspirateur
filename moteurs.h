#ifndef MOTEURS_H
#define MOTEURS_H

void moteursInit();
void moteursAvancer(int vitesse = 180);
void moteursReculer(int vitesse = 150);
void moteursTournerGauche(int vitesse = 170);
void moteursTournerDroite(int vitesse = 170);
void moteursStop();

#endif
