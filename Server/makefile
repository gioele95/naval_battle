#makefile

CFLAGS=-Wall

default: battle_server

battle_server.o: battle_server.c 
	gcc -c battle_server.c -o battle_server.o

battle_server: battle_server.o
	gcc battle_server.o -o battle_server

clean:
	-rm -f battle_server.o
	-rm -f battle_server

