#ifndef STOCKAGE_H
#define STOCKAGE_H

#include <Arduino.h>

// Initialise le système de fichiers LittleFS (à appeler dans setup())
void stockageInit();

// Sauvegarde la carte courante dans /maps/<nomPiece>.bin
// Retourne true si la sauvegarde a réussi
bool stockageSauvegarderCarte(const char* nomPiece);

// Charge une carte depuis /maps/<nomPiece>.bin dans la grille courante
// Retourne true si le chargement a réussi
bool stockageChargerCarte(const char* nomPiece);

// Retourne la liste des pièces sauvegardées sous forme de tableau JSON
// Ex: ["Cuisine","Salon"]
String stockageListerPieces();

#endif
