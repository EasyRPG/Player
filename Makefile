#makefile for easyrpg's player

CC = g++ -g
CFLAGS = `sdl-config --cflags`
#Usually SDL's CFLAGS -I/usr/include/SDL -D_REENTRANT -g
LDFLAGS = `sdl-config --libs` -lSDL_image
#Usually SDL's LDFLAGS -L/usr/lib -lSDL -lpthread -lSDL_image -g


all: main.o tools.o map.o chipset.o
	$(CC) $(LDFLAGS) main.o tools.o map.o chipset.o -o easyRPGPlayer
main.o: tools.h map.h
	$(CC) $(CFLAGS) -c main.cpp
tools.o: tools.h
	$(CC) $(CFLAGS) -c tools.cpp
map.o: map.h tools.h
	$(CC) $(CFLAGS) -c map.cpp
chipset.o: chipset.h tools.h
	$(CC) $(CFLAGS) -c chipset.cpp
clean:
	rm *.o easyRPGPlayer

