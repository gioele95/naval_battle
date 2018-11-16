CC = gcc
CFLAGS = -Wall
DEPS = ../connection/connectionUtility.h UserInterface/front.c UserInterface/front.h battle_client.c
OBJ = battle_client.o connectionUtility.o front.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

battle_client: $(OBJ)
	gcc $(CFLAGS) -o $@ $^