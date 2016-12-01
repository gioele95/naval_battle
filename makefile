#makefile

CC=gcc
CFLAGS=-Wall

p: battle_server.o 
	cc -o p battle_server.o
c: battle_client.o 
	cc -o p battle_client.o
clean:
	rm -f p battle_server.o
	rm -f c battle_client.o



