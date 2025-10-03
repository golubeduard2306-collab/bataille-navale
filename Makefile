# Makefile pour le jeu de bataille navale

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = 

# Fichiers sources
SERVER_SRC = serveur.c
CLIENT_SRC = client.c

# Exécutables
SERVER = serveur
CLIENT = client

# Cibles
all: $(SERVER) $(CLIENT)

# Compilation du serveur
$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC) $(LDFLAGS)

# Compilation du client
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC) $(LDFLAGS)

# Nettoyage des fichiers compilés
clean:
	rm -f $(SERVER) $(CLIENT)

# Nettoyage complet (inclut les fichiers objets si présents)
fclean: clean
	rm -f *.o *~

# Recompilation complète
re: fclean all

# Test du serveur (lance le serveur sur le port 8080)
test-server: $(SERVER)
	./$(SERVER) 8080

# Test du client (se connecte en local)
test-client: $(CLIENT)
	./$(CLIENT) 127.0.0.1 8080

# Aide
help:
	@echo "Makefile pour bataille navale"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  make          - Compile serveur et client"
	@echo "  make all      - Compile serveur et client"
	@echo "  make serveur  - Compile uniquement le serveur"
	@echo "  make client   - Compile uniquement le client"
	@echo "  make clean    - Supprime les exécutables"
	@echo "  make fclean   - Supprime tout (exécutables + fichiers temporaires)"
	@echo "  make re       - Recompile tout"
	@echo "  make test-server - Lance le serveur"
	@echo "  make test-client - Lance le client"
	@echo "  make help     - Affiche cette aide"

# Déclarer les cibles qui ne sont pas des fichiers
.PHONY: all clean fclean re test-server test-client help

