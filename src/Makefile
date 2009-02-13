SHELL = /bin/sh
TARGET = ../easyrpg
CC = g++
SRCS =  attributes/*.cpp control/*.cpp control/events/*.cpp interface/*.cpp interface/Control/*.cpp interface/Windows/*.cpp readers/*.cpp sprites/*.cpp tools/*.cpp *.cpp
CFLAGS = -Os -s -Wall -Wextra -ansi -pedantic $(shell sdl-config --cflags)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

LIBS = -lSDL_mixer -lSDL_gfx -lSDL_ttf -lSDL_image $(shell sdl-config --libs)

all:
	$(CC) -o $(TARGET) $(SRCS) $(CXXFLAGS) $(LIBS)
clean:
	rm -f $(TARGET)
