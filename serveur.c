// serveur.c - Serveur de bataille navale TCP multi-clients
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>  
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define TAILLE_GRILLE 5
#define NB_COUPS_MAX 50
#define NB_PORTE_AVIONS 3
#define NB_FREGATES 5
#define TAILLE_PORTE_AVION 5
#define TAILLE_FREGATE 3

// Structure pour un bateau
typedef struct {
    int x, y;           // Position de départ
    int taille;         // Taille du bateau
    int horizontal;     // 1 si horizontal, 0 si vertical
    int touches;        // Nombre de cases touchées
} Bateau;

// Structure de la grille de jeu
typedef struct {
    char grille[TAILLE_GRILLE][TAILLE_GRILLE];
    Bateau porte_avions[NB_PORTE_AVIONS];
    Bateau fregates[NB_FREGATES];
    int nb_bateaux_restants;
} Jeu;

// Gestionnaire de signal pour les processus fils
void sigchld_handler(int sig) {
    (void)sig;
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// Initialiser la grille
void init_grille(Jeu *jeu) {
    for(int i = 0; i < TAILLE_GRILLE; i++) {
        for(int j = 0; j < TAILLE_GRILLE; j++) {
            jeu->grille[i][j] = '.';
        }
    }
    jeu->nb_bateaux_restants = NB_PORTE_AVIONS + NB_FREGATES;
}

// Vérifier si on peut placer un bateau
int peut_placer(Jeu *jeu, int x, int y, int taille, int horizontal) {
    if(horizontal) {
        if(y + taille > TAILLE_GRILLE) return 0;
        for(int j = y; j < y + taille; j++) {
            if(jeu->grille[x][j] != '.') return 0;
        }
    } else {
        if(x + taille > TAILLE_GRILLE) return 0;
        for(int i = x; i < x + taille; i++) {
            if(jeu->grille[i][y] != '.') return 0;
        }
    }
    return 1;
}

// Placer un bateau sur la grille
void placer_bateau(Jeu *jeu, Bateau *bateau, char symbole) {
    bateau->touches = 0;
    if(bateau->horizontal) {
        for(int j = bateau->y; j < bateau->y + bateau->taille; j++) {
            jeu->grille[bateau->x][j] = symbole;
        }
    } else {
        for(int i = bateau->x; i < bateau->x + bateau->taille; i++) {
            jeu->grille[i][bateau->y] = symbole;
        }
    }
}

// Générer les bateaux aléatoirement
void generer_bateaux(Jeu *jeu) {
    srand(getpid() + time(NULL));
    
    // Placer les porte-avions
    for(int i = 0; i < NB_PORTE_AVIONS; i++) {
        int place = 0;
        while(!place) {
            jeu->porte_avions[i].x = rand() % TAILLE_GRILLE;
            jeu->porte_avions[i].y = rand() % TAILLE_GRILLE;
            jeu->porte_avions[i].taille = TAILLE_PORTE_AVION;
            jeu->porte_avions[i].horizontal = rand() % 2;
            
            if(peut_placer(jeu, jeu->porte_avions[i].x, jeu->porte_avions[i].y,
                          TAILLE_PORTE_AVION, jeu->porte_avions[i].horizontal)) {
                placer_bateau(jeu, &jeu->porte_avions[i], 'P');
                place = 1;
            }
        }
    }
    
    // Placer les frégates
    for(int i = 0; i < NB_FREGATES; i++) {
        int place = 0;
        while(!place) {
            jeu->fregates[i].x = rand() % TAILLE_GRILLE;
            jeu->fregates[i].y = rand() % TAILLE_GRILLE;
            jeu->fregates[i].taille = TAILLE_FREGATE;
            jeu->fregates[i].horizontal = rand() % 2;
            
            if(peut_placer(jeu, jeu->fregates[i].x, jeu->fregates[i].y,
                          TAILLE_FREGATE, jeu->fregates[i].horizontal)) {
                placer_bateau(jeu, &jeu->fregates[i], 'F');
                place = 1;
            }
        }
    }
}

// Vérifier si un bateau est coulé
int est_coule(Bateau *bateau) {
    return bateau->touches >= bateau->taille;
}

// Traiter un tir
char* traiter_tir(Jeu *jeu, int x, int y) {
    static char reponse[256];
    
    // Vérifier les limites
    if(x < 0 || x >= TAILLE_GRILLE || y < 0 || y >= TAILLE_GRILLE) {
        strcpy(reponse, "ERREUR");
        return reponse;
    }
    
    char case_actuelle = jeu->grille[x][y];
    
    // Case déjà jouée
    if(case_actuelle == 'X' || case_actuelle == 'O') {
        strcpy(reponse, "DEJA_JOUE");
        return reponse;
    }
    
    // Raté
    if(case_actuelle == '.') {
        jeu->grille[x][y] = 'O';
        strcpy(reponse, "RATE");
        return reponse;
    }
    
    // Touché
    jeu->grille[x][y] = 'X';
    
    // Chercher quel bateau est touché
    Bateau *bateau_touche = NULL;
    char type = case_actuelle;
    
    if(type == 'P') {
        for(int i = 0; i < NB_PORTE_AVIONS; i++) {
            Bateau *b = &jeu->porte_avions[i];
            if(b->horizontal) {
                if(x == b->x && y >= b->y && y < b->y + b->taille) {
                    bateau_touche = b;
                    break;
                }
            } else {
                if(y == b->y && x >= b->x && x < b->x + b->taille) {
                    bateau_touche = b;
                    break;
                }
            }
        }
    } else if(type == 'F') {
        for(int i = 0; i < NB_FREGATES; i++) {
            Bateau *b = &jeu->fregates[i];
            if(b->horizontal) {
                if(x == b->x && y >= b->y && y < b->y + b->taille) {
                    bateau_touche = b;
                    break;
                }
            } else {
                if(y == b->y && x >= b->x && x < b->x + b->taille) {
                    bateau_touche = b;
                    break;
                }
            }
        }
    }
    
    if(bateau_touche) {
        bateau_touche->touches++;
        
        if(est_coule(bateau_touche)) {
            jeu->nb_bateaux_restants--;
            
            if(jeu->nb_bateaux_restants == 0) {
                strcpy(reponse, "GAGNE");
            } else {
                sprintf(reponse, "COULE:%d", jeu->nb_bateaux_restants);
            }
        } else {
            strcpy(reponse, "TOUCHE");
        }
    } else {
        strcpy(reponse, "TOUCHE");
    }
    
    return reponse;
}

// Gérer un client
void gerer_client(int client_sock, struct sockaddr_in client_addr) {
    Jeu jeu;
    char buffer[256];
    int nb_coups = 0;
    
    printf("Nouveau joueur connecté: %s:%d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    
    // Initialiser le jeu
    init_grille(&jeu);
    generer_bateaux(&jeu);
    
    // Envoyer message de bienvenue
    snprintf(buffer, sizeof(buffer),
             "BIENVENUE:Grille=%dx%d,Coups=%d,PA=%d,F=%d\n",
             TAILLE_GRILLE, TAILLE_GRILLE, NB_COUPS_MAX,
             NB_PORTE_AVIONS, NB_FREGATES);
    write(client_sock, buffer, strlen(buffer));
    
    // Boucle de jeu
    while(nb_coups < NB_COUPS_MAX) {
        int ret = read(client_sock, buffer, sizeof(buffer)-1);
        
        if(ret <= 0) {
            if(ret == 0) {
                printf("Client déconnecté\n");
            } else {
                perror("Erreur lecture");
            }
            break;
        }
        
        buffer[ret] = '\0';
        
        // Parser les coordonnées
        int x, y;
        if(sscanf(buffer, "%d,%d", &x, &y) != 2) {
            write(client_sock, "ERREUR:Format invalide\n", 24);
            continue;
        }
        
        nb_coups++;
        
        // Traiter le tir
        char *resultat = traiter_tir(&jeu, x, y);
        
        snprintf(buffer, sizeof(buffer), "%s:%d\n", resultat, nb_coups);
        write(client_sock, buffer, strlen(buffer));
        
        printf("Tir reçu (%d,%d) -> %s\n", x, y, resultat);
        
        // Vérifier fin de partie
        if(strcmp(resultat, "GAGNE") == 0) {
            printf("Partie gagnée en %d coups!\n", nb_coups);
            break;
        }
    }
    
    // Fin de partie
    if(nb_coups >= NB_COUPS_MAX && jeu.nb_bateaux_restants > 0) {
        write(client_sock, "PERDU\n", 6);
        printf("Partie perdue (nombre de coups dépassé)\n");
    }
    
    close(client_sock);
    printf("Connexion fermée\n");
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = PORT;
    
    // Argument optionnel pour le port
    if(argc > 1) {
        port = atoi(argv[1]);
    }
    
    // Configuration du gestionnaire de signaux
    signal(SIGCHLD, sigchld_handler);
    
    // Création socket
    if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erreur socket");
        exit(1);
    }
    
    // Option pour réutiliser l'adresse
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configuration adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Liaison
    if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_sock);
        exit(1);
    }
    
    // Écoute
    if(listen(server_sock, 5) != 0) {
        perror("Erreur listen");
        close(server_sock);
        exit(1);
    }
    
    printf("Serveur de bataille navale démarré sur le port %d\n", port);
    printf("En attente de joueurs...\n");
    
    // Boucle principale
    while(1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        
        if(client_sock < 0) {
            perror("Erreur accept");
            continue;
        }
        
        // Fork pour gérer le client
        pid_t pid = fork();
        
        if(pid < 0) {
            perror("Erreur fork");
            close(client_sock);
            continue;
        }
        
        if(pid == 0) {
            // Processus fils
            close(server_sock);
            gerer_client(client_sock, client_addr);
            exit(0);
        } else {
            // Processus père
            close(client_sock);
        }
    }
    
    close(server_sock);
    return 0;
}