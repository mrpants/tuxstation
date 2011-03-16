#
# Makefile for lemonlauncher
#
CC=gcc
CFLAGS=-g -Wall -O2 -pedantic -s
SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs) -lSDL_image -lSDL_mixer -lSDL_ttf
SRC=tuxstation.c parsecfg.c
OBJ=tuxstation.o parsecfg.o

all: tuxstation

clean: $(OBJ)
	rm $(OBJ) tuxstation

tuxstation: $(OBJ)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(SDL_LDFLAGS) -o tuxstation $(OBJ)

tuxstation.o:tuxstation.c tuxconfig.h parsecfg.c

parsecfg.o:parsecfg.c parsecfg.h

