#DFLAGS=
INCLUDE=.
LIBS=-lSDL2 -lGL -lGLEW -lm
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=c99 -pedantic -I$(INCLUDE)
MAKE=make

TARGET=cgame
OBJECTS=main.o window.o error.o sprite.o vertex.o gl_program.o \
		file_buf.o upng/upng.o texture.o camera.o math_lib/math_lib.a \
		inmgr.o game.o

all: $(OBJECTS) Makefile math_lib/mat4f.h
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

math_lib/math_lib.a: math_lib/Makefile
	$(MAKE) -C math_lib

%o: %.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<
clean:
	rm $(OBJECTS) $(TARGET)
