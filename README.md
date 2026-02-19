# robot-aspirateur (ESP32 cartographe)

Ce dépôt contient une base **modulaire Arduino/ESP32** orientée SLAM léger par grille d’occupation.

## Architecture logicielle

- `RobotAspirateur.ino` : orchestration FreeRTOS (cartographie, navigation, communication)
- `carte.*` : grille d’occupation en chunks + pose du robot
- `odometrie.*` : encodeurs + ISR + conversion tics→distance
- `imu.*` : lecture yaw (stub simulation prêt à brancher MPU6050/6500)
- `ultrasons.*` : télémétrie obstacle avant (HC-SR04)
- `navigation.*` : décision de mouvement à partir carte + obstacle + batterie
- `moteurs.*`, `aspiration.*`, `batterie.*` : actionneurs et sécurité énergie
- `config.h` : toutes les constantes matérielles / périodes de tâches

## Conception

- Carte : `100 x 100` chunks (`5 cm/chunk`) => 10 000 cellules
- Valeurs cellule : `0` inconnu, `1` libre, `2` obstacle
- Mise à jour pose : trigonométrie (`x += d*cos(yaw)`, `y += d*sin(yaw)`)
- Sans `delay()` bloquant dans le flux principal : multitâche via FreeRTOS

## Tâches FreeRTOS

- `TacheCartographie` (50 Hz, core 0) : odométrie + IMU + ultrasons + update carte
- `TacheNavigation` (20 Hz, core 1) : choisit l’action et pilote moteurs/aspiration
- `TacheCommunication` (2 Hz, core 1) : publie l’état (actuellement Serial)

## Mode simulation

`MODE_SIMULATION=true` permet d’exécuter la logique sans capteurs réels.
Quand le matériel est prêt, remplacez progressivement les stubs dans `imu.cpp` / `ultrasons.cpp` et ajustez `config.h`.
