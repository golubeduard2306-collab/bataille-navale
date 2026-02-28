# Bataille Navale

> Jeu de bataille navale en réseau, architecture client-serveur TCP en C.

## Stack technique

- **Langage :** C (POSIX)
- **Réseau :** Sockets BSD — TCP/IP
- **Concurrence :** `fork()` + `SIGCHLD` — serveur multi-clients
- **Build :** GNU Make / GCC

## Fonctionnalités

- Serveur gérant plusieurs clients simultanés via le fork de processus
- Grille 5×5 générée aléatoirement avec porte-avions (5 cases) et frégates (3 cases)
- Retour en temps réel sur chaque tir : RATE / TOUCHE / COULE / GAGNE / PERDU
- Hôte et port configurables via arguments en ligne de commande
- Makefile avec cibles de compilation, nettoyage et test

## Installation & Lancement

**Prérequis :** GCC, Make, système Unix (Linux / WSL / macOS)

```bash
# Cloner le dépôt
git clone https://github.com/egolub23/bataille-navale.git
cd bataille-navale

# Compiler le serveur et le client
make

# Lancer le serveur (port 8080 par défaut)
./serveur 8080

# Dans un autre terminal, connecter un client
./client 127.0.0.1 8080
```

## Utilisation

Une fois connecté, saisir les coordonnées au format `x,y` (ex. `2,3`) pour tirer.
Taper `quit` pour se déconnecter.

```
Connexion au serveur 127.0.0.1:8080...
Connecté!

=== BATAILLE NAVALE ===
Coordonnées (x,y) ou 'quit': 2,3
💥 Touché! Bon tir!
```

## Contexte

IUT La Rochelle — BUT Informatique — 2025
Module Réseaux & Programmation Système — Semestre 3
