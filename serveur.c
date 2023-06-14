#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define NAME_LENGTH 32
#define PORT 30004

typedef struct {
    int socket;
    char name[NAME_LENGTH];
} Client;

int client_count = 0;
Client client_sockets[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char client_names[MAX_CLIENTS][NAME_LENGTH];

void send_message_all(char* message, int current_client) {
    pthread_mutex_lock(&mutex); // Verrouillage du mutex pour assurer une exécution exclusive de cette section critique

    // Parcours de tous les clients connectés
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket != current_client) { // Si le client actuel n'est pas le client qui a envoyé le message
            send(client_sockets[i].socket, message, strlen(message), 0); // Envoi du message au client actuel
        }
    }

    pthread_mutex_unlock(&mutex); // Déverrouillage du mutex pour permettre à d'autres threads d'accéder à cette section critique
}

void send_message_to_client(char* message, int client_socket) {
    pthread_mutex_lock(&mutex); // Verrouillage du mutex pour assurer une exécution exclusive de cette section critique

    send(client_socket, message, strlen(message), 0); // Envoi du message au client spécifié

    pthread_mutex_unlock(&mutex); // Déverrouillage du mutex pour permettre à d'autres threads d'accéder à cette section critique
}

void send_client_list(int client_socket) {
    char client_list[BUFFER_SIZE]; // Buffer pour stocker la liste des clients connectés
    memset(client_list, 0, BUFFER_SIZE); // Initialisation du buffer à zéro pour éviter les données indésirables

    // Parcourir la liste des clients connectés
    for (int i = 0; i < client_count; i++) {
        strcat(client_list, client_names[i]); // Ajouter le nom du client à la liste
        strcat(client_list, "\n"); // Ajouter un saut de ligne après chaque nom de client
    }

    send_message_to_client(client_list, client_socket); // Envoyer la liste des clients au client spécifié
}

void *handle_client(void *arg) {
    Client client = *((Client *)arg);
    char buffer[BUFFER_SIZE];
    char welcome_message[NAME_LENGTH + 28];
    sprintf(welcome_message, "Bienvenue, %s !\n", client.name);
    
    send(client.socket, welcome_message, strlen(welcome_message), 0);
    printf("Nouvelle connexion établie. Nom client : %s\n", client.name);
    
    pthread_mutex_lock(&mutex);
    client_sockets[client_count++] = client;

    strcpy(client_names[client_count - 1], client.name); // Ajouter le nom du client à la liste des noms

    pthread_mutex_unlock(&mutex);
    
    while (1) {
        int message_length = recv(client.socket, buffer, BUFFER_SIZE, 0);
        if (message_length <= 0) {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; i++) {
                if (client_sockets[i].socket == client.socket) {
                    printf("Déconnexion. Nom client : %s\n", client.name);
                    close(client.socket);
                    while (i < client_count - 1) {
                        client_sockets[i] = client_sockets[i + 1];
                        i++;
                    }
                    client_count--;

                    memset(client_names[i], 0, NAME_LENGTH); // Supprimer le nom du client de la liste des noms

                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        buffer[message_length] = '\0';

        // Envoyer la liste des noms des clients connectés
        if (strcmp(buffer, "#list") == 0) {
            send_client_list(client.socket);
            continue; // Passer à la prochaine itération pour éviter l'envoi du message à tous les clients
        }

        // Vérifier si le message est destiné à un client spécifique
        if (buffer[0] == '@') {
            char* recipient_name = strtok(buffer, " ");
            char* message = strtok(NULL, "\0");
            if (recipient_name != NULL && message != NULL) {
                // Rechercher le client destinataire
                int recipient_socket = -1;
                pthread_mutex_lock(&mutex);
                for (int i = 0; i < client_count; i++) {
                    if (strcmp(client_sockets[i].name, recipient_name + 1) == 0) {
                        recipient_socket = client_sockets[i].socket;
                        break;
                    }
                }
                pthread_mutex_unlock(&mutex);
                
                // Envoyer le message au client destinataire
                if (recipient_socket != -1) {
                    char message_with_name[NAME_LENGTH + BUFFER_SIZE + 4];
                    sprintf(message_with_name, "%s : %s", client.name, message);
                    send_message_to_client("(Uniquement à vous :) ", recipient_socket);  // Message spécifique destiné au client destinataire
                    send_message_to_client(message_with_name, recipient_socket);
                } else {
                    send_message_to_client("Le destinataire n'existe pas.\n", client.socket);
                }
            }
        } else {
            // Envoyer le message à tous les clients
            char message_with_name[NAME_LENGTH + BUFFER_SIZE + 3];
            sprintf(message_with_name, "%s : %s", client.name, buffer);
            send_message_all(message_with_name, client.socket);
        }
    }
    
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    pthread_t thread_id;
    
    // Création du socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket serveur");
        exit(EXIT_FAILURE);
    }
    
    // Configuration de l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    
    // Lien du socket à l'adresse et au port spécifiés
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Erreur lors du lien du socket serveur");
        exit(EXIT_FAILURE);
    }
    
    // Écoute des connexions entrantes
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Erreur lors de l'écoute des connexions entrantes");
        exit(EXIT_FAILURE);
    }
    
    printf("Serveur de messagerie lancé. En attente de connexions...\n");
    
    while (1) {
        socklen_t client_address_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        
        if (client_socket == -1) {
            perror("Erreur lors de la tentative d'acceptation d'une connexion");
            exit(EXIT_FAILURE);
        }
        
        // Recevoir le nom du client
        char client_name[NAME_LENGTH];
        int name_length = recv(client_socket, client_name, NAME_LENGTH, 0);
        if (name_length <= 0) {
            perror("Erreur lors de la réception du nom du client");
            exit(EXIT_FAILURE);
        }
        client_name[name_length] = '\0';
        
        // Création de la structure Client
        Client client;
        client.socket = client_socket;
        strncpy(client.name, client_name, NAME_LENGTH);
        
        pthread_create(&thread_id, NULL, handle_client, &client);
        pthread_detach(thread_id);
    }
    
    close(server_socket);
    
    return 0;
}