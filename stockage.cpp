#include <Arduino.h>
#include <LittleFS.h>
#include "stockage.h"
#include "carte.h"
#include "config.h"
#include "debug.h"

// -----------------------------------------------------------------------
// Helpers internes
// -----------------------------------------------------------------------

// Construit le chemin complet d'un fichier carte : /maps/<nom>.bin
static String cheminFichier(const char* nomPiece) {
  return "/maps/" + String(nomPiece) + ".bin";
}

// -----------------------------------------------------------------------
// API publique
// -----------------------------------------------------------------------

void stockageInit() {
  if (!LittleFS.begin(true)) {
    debugLog("[STOCKAGE] ERREUR: LittleFS impossible a demarrer !");
    return;
  }
  // Crée le dossier /maps s'il n'existe pas encore
  if (!LittleFS.exists("/maps")) {
    LittleFS.mkdir("/maps");
  }
  debugLog("[STOCKAGE] LittleFS OK");
}

bool stockageSauvegarderCarte(const char* nomPiece) {
  String chemin = cheminFichier(nomPiece);
  File f = LittleFS.open(chemin, "w");
  if (!f) {
    debugLog("[STOCKAGE] ERREUR: impossible de creer " + chemin);
    return false;
  }

  const int larg = carteLargeur();
  const int long_ = carteLongueur();

  // En-tête : dimensions (2 × int)
  f.write(reinterpret_cast<const uint8_t*>(&larg),  sizeof(int));
  f.write(reinterpret_cast<const uint8_t*>(&long_), sizeof(int));

  // Grille aplatie, ligne par ligne
  for (int x = 0; x < larg; x++) {
    for (int y = 0; y < long_; y++) {
      uint8_t val = carteLireCase(x, y);
      f.write(val);
    }
  }

  f.close();
  debugLog("[STOCKAGE] Carte sauvegardee : " + chemin);
  return true;
}

bool stockageChargerCarte(const char* nomPiece) {
  String chemin = cheminFichier(nomPiece);
  File f = LittleFS.open(chemin, "r");
  if (!f) {
    debugLog("[STOCKAGE] ERREUR: fichier introuvable " + chemin);
    return false;
  }

  int larg = 0, long_ = 0;
  f.read(reinterpret_cast<uint8_t*>(&larg),  sizeof(int));
  f.read(reinterpret_cast<uint8_t*>(&long_), sizeof(int));

  if (larg != carteLargeur() || long_ != carteLongueur()) {
    debugLog("[STOCKAGE] ERREUR: dimensions incompatibles dans " + chemin);
    f.close();
    return false;
  }

  // Remet la grille à zéro sans changer la position courante du robot
  carteInit();

  for (int x = 0; x < larg; x++) {
    for (int y = 0; y < long_; y++) {
      uint8_t val = 0;
      f.read(&val, 1);
      carteSetCase(x, y, val);
    }
  }

  f.close();
  debugLog("[STOCKAGE] Carte chargee : " + chemin);
  return true;
}

String stockageListerPieces() {
  String result = "[";
  bool first = true;

  File dir = LittleFS.open("/maps");
  if (dir && dir.isDirectory()) {
    File f = dir.openNextFile();
    while (f) {
      if (!f.isDirectory()) {
        String name = String(f.name());
        // Retire le préfixe de chemin éventuel
        int lastSlash = name.lastIndexOf('/');
        if (lastSlash >= 0) name = name.substring(lastSlash + 1);
        // Garde uniquement les fichiers .bin
        if (name.endsWith(".bin")) {
          name = name.substring(0, name.length() - 4);
          if (!first) result += ",";
          result += "\"" + name + "\"";
          first = false;
        }
      }
      f = dir.openNextFile();
    }
    dir.close();
  }

  result += "]";
  return result;
}
