CC = g++
CFLAGS = `sdl-config --cflags`
LDFLAGS = `sdl-config --libs` -lSDL_image

all: main.o tools.o map.o chipset.o bitmap.o window.o
	$(CC) main.o tools.o map.o chipset.o bitmap.o window.o -o easyrpg $(LDFLAGS)

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

window.o: window.h bitmap.h window.cpp
	$(CC) $(CFLAGS) -c window.cpp

clean:
	rm *.o easyrpg

