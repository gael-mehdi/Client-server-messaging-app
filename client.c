#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define NAME_LENGTH 32
#define PORT 30005

int server_socket;

void *receive_messages() {
    char buffer[BUFFER_SIZE]; // Tampon pour stocker les messages reçus
    while (1) {
        int message_length = recv(server_socket, buffer, BUFFER_SIZE, 0); // Réception du message du serveur
        if (message_length <= 0) { // Si la longueur du message est nulle ou négative, le serveur s'est déconnecté
            printf("Déconnexion du serveur.\n"); // Affichage de message de déconnexion
            break; // Sortie de la boucle
        }
        buffer[message_length] = '\0'; // Ajout du caractère de fin de chaîne au message reçu
        printf("%s\n", buffer); // Affichage du message reçu
    }
    close(server_socket); // Fermeture du socket du serveur
    exit(EXIT_SUCCESS); // Sortie du programme avec succès
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Utilisation: %s <nom>\n", argv[0]); // Affiche un message d'utilisation si le nombre d'arguments est incorrect
        exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
    }
    
    char *client_name = argv[1]; // Récupère le nom du client à partir des arguments de ligne de commande
    
    struct sockaddr_in server_address; // Structure pour l'adresse du serveur
    pthread_t receive_thread; // Identifiant du thread de réception des messages
    
    // Création du socket client
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // Crée un socket de type TCP/IP
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket client"); // Affiche un message d'erreur en cas d'échec de la création du socket
        exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
    }
    
    // Configuration de l'adresse du serveur
    server_address.sin_family = AF_INET; // Famille d'adresses IPv4
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // Adresse IP du serveur (dans cet exemple, l'adresse est locale)
    server_address.sin_port = htons(PORT);  // Port du serveur (dans cet exemple, PORT est une constante définie ailleurs)
    
    // Connexion au serveur
    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Erreur lors de la tentative de connexion au serveur"); // Affiche un message d'erreur en cas d'échec de la connexion
        exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
    }
    
    // Envoi du nom du client au serveur
    if (send(server_socket, client_name, strlen(client_name), 0) == -1) {
        perror("Erreur lors de l'envoi du nom du client"); // Affiche un message d'erreur en cas d'échec de l'envoi du nom du client
        exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
    }
    
    printf("Connecté au serveur de messagerie.\n");
    
    // Création du thread de réception des messages
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Erreur lors de la création du thread de réception des messages"); // Affiche un message d'erreur en cas d'échec de la création du thread
        exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
    }
    
    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin); // Lit une ligne de l'entrée standard (message à envoyer)
        message[strcspn(message, "\n")] = '\0';  // Supprime le caractère de saut de ligne à la fin du message
        
        // Vérifie si le message est destiné à un client spécifique
        if (message[0] == '@') {
            char* recipient_name = strtok(message, " "); // Extrait le nom du destinataire du message
            char* actual_message = strtok(NULL, "\0"); // Extrait le contenu du message
            if (recipient_name != NULL && actual_message != NULL) {
                // Envoyer le message au serveur avec le préfixe '@' pour indiquer le destinataire
                char message_with_recipient[BUFFER_SIZE + 2];
                sprintf(message_with_recipient, "%s %s", recipient_name, actual_message); // Concatène le nom du destinataire et le message
                if (send(server_socket, message_with_recipient, strlen(message_with_recipient), 0) == -1) {
                    perror("Erreur lors de l'envoi du message"); // Affiche un message d'erreur en cas d'échec de l'envoi du message
                    exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
                }
            }
        } else {
            // Envoyer le message au serveur sans destinataire spécifique
            if (send(server_socket, message, strlen(message), 0) == -1) {
                perror("Erreur lors de l'envoi du message"); // Affiche un message d'erreur en cas d'échec de l'envoi du message
                exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
            }
        }
    }
    
    close(server_socket); // Fermeture du socket du serveur
    
    return 0; // Quitte le programme avec succès
}
