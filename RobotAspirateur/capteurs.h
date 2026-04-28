#ifndef CAPTEURS_H
#define CAPTEURS_H

struct EtatCapteurs {
  bool obstacleDevant;
  bool videDetecte;
};

void capteursInit();
EtatCapteurs capteursLire();

#endif
