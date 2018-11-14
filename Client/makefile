#makefile

CC=gcc
CFLAGS=-Wall
all:server client

server: battle_server.o 
	cc -o server battle_server.o
client: battle_client.o 
	cc -o client battle_client.o
clean:
	rm -f p battle_server.o
	rm -f c battle_client.o



