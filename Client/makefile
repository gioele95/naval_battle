#makefile

CFLAGS=-Wall

default: battle_client

battle_client.o: battle_client.c 
	gcc -c battle_client.c -o battle_client.o

battle_client: battle_client.o
	gcc battle_client.o -o battle_client

clean:
	-rm -f battle_client.o
	-rm -f battle_client

