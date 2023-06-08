#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define NAME_LENGTH 32

int server_socket;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int message_length = recv(server_socket, buffer, BUFFER_SIZE, 0);
        if (message_length <= 0) {
            printf("Déconnexion du serveur.\n");
            break;
        }
        buffer[message_length] = '\0';
        printf("%s\n", buffer);
    }
    close(server_socket);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Utilisation: %s <nom>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char *client_name = argv[1];
    
    struct sockaddr_in server_address;
    pthread_t receive_thread;
    
    // Création du socket client
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket client");
        exit(EXIT_FAILURE);
    }
    
    // Configuration de l'adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // Adresse IP du serveur
    server_address.sin_port = htons(12345);  // Port du serveur
    
    // Connexion au serveur
    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Erreur lors de la tentative de connexion au serveur");
        exit(EXIT_FAILURE);
    }
    
    // Envoi du nom du client au serveur
    if (send(server_socket, client_name, strlen(client_name), 0) == -1) {
        perror("Erreur lors de l'envoi du nom du client");
        exit(EXIT_FAILURE);
    }
    
    printf("Connecté au serveur de messagerie.\n");
    
    // Création du thread de réception des messages
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Erreur lors de la création du thread de réception des messages");
        exit(EXIT_FAILURE);
    }
    
    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';  // Supprimer le saut de ligne
        
        // Vérifier si le message est destiné à un client spécifique
        if (message[0] == '@') {
            char* recipient_name = strtok(message, " ");
            char* actual_message = strtok(NULL, "\0");
            if (recipient_name != NULL && actual_message != NULL) {
                // Envoyer le message au serveur avec le préfixe '@' pour indiquer le destinataire
                char message_with_recipient[BUFFER_SIZE + 2];
                sprintf(message_with_recipient, "%s %s", recipient_name, actual_message);
                if (send(server_socket, message_with_recipient, strlen(message_with_recipient), 0) == -1) {
                    perror("Erreur lors de l'envoi du message");
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            // Envoyer le message au serveur sans destinataire spécifique
            if (send(server_socket, message, strlen(message), 0) == -1) {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    close(server_socket);
    
    return 0;
}
