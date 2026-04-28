# Présentation du projet – Robot aspirateur autonome

## Architecture générale & approche logicielle

## 1️⃣ Objectif du projet

Le projet consiste à concevoir un **robot aspirateur autonome**, comprenant :

- un **système d’aspiration complet** (fente, chambre de déchets, filtres, turbine),
- un **système de déplacement motorisé** (2 roues motrices),
- une **alimentation sur batterie**,
- et un **pilotage par microcontrôleur Arduino**.

Le développement est mené de manière **progressive et modulaire**, en séparant clairement :

- la **conception mécanique**,
- la **logique logicielle**,
- et l’**intégration matérielle**.

## 2️⃣ Principe fondamental : séparer le logiciel du matériel

Un point clé du projet est de **coder avant même d’avoir les composants physiques**.

Pour cela :

- on écrit le programme comme si le robot existait déjà,
- on remplace temporairement les composants par des **fonctions simulées**,
- et le jour où les composants arrivent, on adapte uniquement le code bas niveau (pins, lectures, commandes).

👉 Cela permet :

- de gagner du temps,
- d’avoir un code propre et professionnel,
- d’éviter de tout refaire à la fin.

## 3️⃣ Environnement de développement

- **Microcontrôleur** : Arduino (Uno, Nano ou équivalent)
- **Langage** : C++ Arduino
- **IDE** : Arduino IDE

Le langage Arduino est du C++ simplifié, parfaitement adapté à un projet robotique modulaire.

## 4️⃣ Vision globale du fonctionnement du robot

Le robot fonctionne selon une **boucle de contrôle continue** :

1. Initialisation des composants
2. Lecture des capteurs
3. Prise de décision
4. Action sur les moteurs et l’aspiration
5. Vérification de la batterie
6. Répétition de la boucle

C’est ce qu’on appelle une **machine à états**.

## 5️⃣ Architecture logicielle (modulaire)

Le projet est découpé en **modules indépendants**, chacun responsable d’une fonction précise.

### Structure logique du projet

```text
RobotAspirateur/
│
├── RobotAspirateur.ino   → programme principal (cerveau)
│
├── config.h              → constantes globales (pins, seuils)
│
├── moteurs.h / moteurs.cpp
├── aspiration.h / aspiration.cpp
├── capteurs.h / capteurs.cpp
├── batterie.h / batterie.cpp
├── navigation.h / navigation.cpp
└── debug.h
```

Chaque module :

- a une responsabilité unique,
- peut être développé et testé indépendamment,
- peut évoluer sans casser le reste du projet.

## 6️⃣ Programme principal (cerveau du robot)

Le fichier principal ne contient **aucune logique complexe**.
Il se contente de :

- démarrer les modules,
- appeler les fonctions principales dans la boucle.

Exemple de logique :

- initialiser moteurs, aspiration, capteurs, batterie ;
- dans la boucle :
  - lire les capteurs,
  - décider du comportement,
  - piloter les moteurs,
  - activer ou non l’aspiration,
  - surveiller la batterie.

👉 Cela rend le code **très lisible et maintenable**.

## 7️⃣ Modules fonctionnels

### 🔹 Module moteurs

Responsable du déplacement :

- avancer,
- reculer,
- tourner à gauche / droite,
- s’arrêter.

La logique de déplacement est indépendante du câblage réel.

### 🔹 Module aspiration

Responsable du système d’aspiration :

- démarrage / arrêt de la turbine,
- (plus tard) gestion de puissance,
- (plus tard) sécurité si filtre bouché ou batterie faible.

C’est un module central du projet.

### 🔹 Module capteurs

Regroupe toutes les entrées :

- détection d’obstacles,
- détection de vide,
- autres capteurs futurs.

Même sans capteurs réels, les fonctions peuvent être simulées.

### 🔹 Module batterie

Gère l’énergie :

- lecture de la tension,
- détection de batterie faible,
- déclenchement de sécurités (arrêt aspiration, arrêt moteurs).

### 🔹 Module navigation (logique décisionnelle)

Décide :

- quand avancer,
- quand tourner,
- quand s’arrêter,
- comment réagir aux obstacles.

Il utilise les informations des capteurs, mais ne dépend pas de leur implémentation physique.

## 8️⃣ Simulation avant intégration

Avant d’avoir le robot :

- les fonctions capteurs retournent des valeurs fixes ou simulées,
- les moteurs n’actionnent rien physiquement,
- mais **toute la logique est déjà écrite et testée**.

Le jour de l’intégration :

- on remplace la simulation par le vrai code matériel,
- sans toucher à l’architecture globale.

## 9️⃣ Philosophie du projet

Ce projet est conçu comme un **vrai projet d’ingénierie** :

- structuré,
- évolutif,
- compréhensible par un tiers,
- prêt à être amélioré (retour à la base, cartographie, etc.).

Chaque choix vise :

- la clarté,
- la fiabilité,
- et la montée en compétence.

## 🔚 Conclusion

Le robot aspirateur est pensé comme :

- un **système modulaire complet**,
- piloté par une **architecture logicielle propre**,
- permettant de travailler en parallèle sur la mécanique, l’électronique et le code.

Cette approche garantit un projet :

- réaliste,
- pédagogique,
- et professionnel.
