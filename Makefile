CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: serveur client

serveur: serveur.c
	$(CC) $(CFLAGS) $< -o $@

client: client.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f serveur client
