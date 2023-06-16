# Messagerie Client-Serveur

Ce projet est une application de messagerie client-serveur. L'application permet à plusieurs clients de se connecter à un serveur et d'envoyer des messages les uns aux autres.

## Description du sujet

L'objectif de ce projet est de développer un système de messagerie permettant à plusieurs utilisateurs de communiquer entre eux via un serveur centralisé. Chaque client peut se connecter au serveur en fournissant son nom, puis envoyer des messages à tous les utilisateurs connectés ou à un utilisateur spécifique.

Le projet est divisé en deux parties principales :
- Le serveur de messagerie : il gère les connexions des clients, les messages entrants et sortants, ainsi que la gestion des utilisateurs connectés.
- Le client de messagerie : il permet à un utilisateur de se connecter au serveur et d'envoyer/recevoir des messages.

## Organisation des programmes

Le projet est organisé en plusieurs fichiers :

- `serveur.c` : le code source du serveur de messagerie.
- `client.c` : le code source du client de messagerie.
- `Makefile` : un fichier de configuration pour la compilation et l'exécution du projet.

## Mode opératoire

### Installation

Décompresser l'archive tar avec la commande : "tar -xvf Projet_LeLay_Levasseur.tar".

### Compilation

1. Accédez au répertoire contenant les fichiers du projet.

2. Ouvrez un terminal et exécutez la commande suivante pour compiler le serveur de messagerie : 'make'.

### Exécution

1. Pour utiliser l'application, exécuter dans un premier terminal le serveur avec la commande : './serveur'.

2. Ensuite, lancer les clients avec la commande : './client nom_du_client', avec nom le nom du client.

Vous pouvez maintenant échanger des messages.

3. Les messages que vous envoyer sont diffusés à tous les clients.
Vous pouvez connaître la liste des clients avec le message "#list". La liste des clients connectés s'affichera dans votre terminal.
Pour envoyer un message à un unique client, écrire : "@nom_destinataire message".
