#makefile for easyrpg's player

CC = g++
CFLAGS = `sdl-config --cflags`
#Usually SDL's CFLAGS -I/usr/include/SDL -D_REENTRANT -g
LDFLAGS = `sdl-config --libs` -lSDL_image
#Usually SDL's LDFLAGS -L/usr/lib -lSDL -lpthread -lSDL_image -g


all: main.o tools.o map.o chipset.o bitmap.o
	$(CC) main.o tools.o map.o chipset.o bitmap.o -o easyRPGPlayer $(LDFLAGS)
main.o: tools.h map.h bitmap.h main.cpp
	$(CC) $(CFLAGS) -c main.cpp
tools.o: tools.h tools.cpp
	$(CC) $(CFLAGS) -c tools.cpp
map.o: map.h tools.h map.cpp
	$(CC) $(CFLAGS) -c map.cpp
chipset.o: chipset.h tools.h chipset.cpp
	$(CC) $(CFLAGS) -c chipset.cpp
bitmap.o: bitmap.h tools.h bitmap.cpp
	$(CC) $(CFLAGS) -c bitmap.cpp
clean:
	rm *.o easyRPGPlayer

