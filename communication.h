#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "carte.h"
#include "modes.h"

void communicationInit();

// Envoi de la télémétrie vers tous les clients WebSocket connectés
void communicationEnvoyerMiseAJour(PositionRobot pos, float distanceObstacle,
                                   float niveauBatterie, bool aspirationActive);

// Envoi de la liste des pièces sauvegardées (tableau JSON)
void communicationEnvoyerListePieces(const String& jsonTableau);

void communicationCleanupClients();

// --- Getters d'état ---
ModeRobot        communicationGetMode();
CommandeManuelle communicationGetCommandeManuelle();
bool             communicationGetAspirationManuelle();

// --- Flags de persistance (sauvegarde carte) ---
// Retourne true si une sauvegarde est en attente et remplit nomOut (taille max : taille)
bool communicationSauvegarderDemande(char* nomOut, size_t taille);
void communicationAcquitterSauvegarde();

// --- Flags de persistance (chargement carte) ---
// Retourne true si un chargement est en attente et remplit nomOut
bool communicationChargerDemande(char* nomOut, size_t taille);
void communicationAcquitterChargement();

// --- Flag liste des pièces ---
bool communicationListePiecesDemandee();
void communicationAcquitterListePieces();

// Setter interne (mode forcé depuis le firmware, ex: fin du nettoyage)
void communicationSetMode(ModeRobot mode);

#endif
