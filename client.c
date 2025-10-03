// client.c - Client de bataille navale TCP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 256

// Afficher les règles du jeu
void afficher_regles() {
    printf("\n=== BATAILLE NAVALE ===\n");
    printf("Entrez les coordonnées de tir au format: x,y\n");
    printf("Exemple: 10,25\n");
    printf("Tapez 'quit' pour quitter\n\n");
}

// Afficher le résultat d'un tir
void afficher_resultat(char *reponse) {
    if(strncmp(reponse, "BIENVENUE", 9) == 0) {
        printf("\n🎮 %s\n", reponse);
        afficher_regles();
    } else if(strcmp(reponse, "RATE") == 0) {
        printf("💧 Raté! Continuez...\n");
    } else if(strcmp(reponse, "TOUCHE") == 0) {
        printf("💥 Touché! Bon tir!\n");
    } else if(strncmp(reponse, "COULE", 5) == 0) {
        int restants;
        sscanf(reponse, "COULE:%d", &restants);
        printf("🚢 Coulé! Il reste %d bateau(x)\n", restants);
    } else if(strcmp(reponse, "GAGNE") == 0) {
        printf("🏆 GAGNÉ! Tous les bateaux sont coulés!\n");
    } else if(strcmp(reponse, "PERDU") == 0) {
        printf("😢 PERDU! Nombre de coups dépassé\n");
    } else if(strcmp(reponse, "DEJA_JOUE") == 0) {
        printf("⚠️  Case déjà jouée!\n");
    } else if(strncmp(reponse, "ERREUR", 6) == 0) {
        printf("❌ Erreur: %s\n", reponse);
    }
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[100];
    char *host = "127.0.0.1";
    int port = PORT;
    
    // Arguments: ./client [host] [port]
    if(argc > 1) {
        host = argv[1];
    }
    if(argc > 2) {
        port = atoi(argv[2]);
    }
    
    // Création socket
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erreur socket");
        exit(1);
    }
    
    // Configuration adresse serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if(inet_aton(host, &server_addr.sin_addr) == 0) {
        fprintf(stderr, "Adresse IP invalide\n");
        close(sock);
        exit(1);
    }
    
    // Connexion au serveur
    printf("Connexion au serveur %s:%d...\n", host, port);
    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur connexion");
        close(sock);
        exit(1);
    }
    
    printf("Connecté!\n");
    
    // Recevoir message de bienvenue
    int ret = read(sock, buffer, BUFFER_SIZE-1);
    if(ret > 0) {
        buffer[ret] = '\0';
        buffer[strcspn(buffer, "\n")] = 0;
        afficher_resultat(buffer);
    }
    
    // Boucle de jeu
    while(1) {
        printf("\nCoordonnées (x,y) ou 'quit': ");
        fflush(stdout);
        
        if(fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Supprimer le retour à la ligne
        input[strcspn(input, "\n")] = 0;
        
        // Vérifier quit
        if(strcmp(input, "quit") == 0) {
            printf("Au revoir!\n");
            break;
        }
        
        // Valider format
        int x, y;
        if(sscanf(input, "%d,%d", &x, &y) != 2) {
            printf("❌ Format invalide! Utilisez: x,y\n");
            continue;
        }
        
        // Envoyer coordonnées
        snprintf(buffer, sizeof(buffer), "%d,%d", x, y);
        if(write(sock, buffer, strlen(buffer)) < 0) {
            perror("Erreur envoi");
            break;
        }
        
        // Recevoir résultat
        ret = read(sock, buffer, BUFFER_SIZE-1);
        if(ret <= 0) {
            if(ret == 0) {
                printf("Serveur déconnecté\n");
            } else {
                perror("Erreur réception");
            }
            break;
        }
        
        buffer[ret] = '\0';
        
        // Parser réponse
        char resultat[50];
        int nb_coups;
        if(sscanf(buffer, "%[^:]:%d", resultat, &nb_coups) >= 1) {
            afficher_resultat(resultat);
            printf("Coups joués: %d\n", nb_coups);
            
            // Vérifier fin de partie
            if(strcmp(resultat, "GAGNE") == 0 || strcmp(resultat, "PERDU") == 0) {
                break;
            }
        }
    }
    
    close(sock);
    return 0;
}