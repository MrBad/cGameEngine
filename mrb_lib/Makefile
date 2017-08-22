#DFLAGS=
INCLUDE=.
LIBS=-lSDL2 -lGL -lGLEW -lm
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=c99 -pedantic -I$(INCLUDE)
MAKE=make

TARGET=mrb_lib.a
OBJECTS=camera.o error.o file_buf.o gl_program.o inmgr.o mat4f.o \
		sprite.o sprite_batch.o texture.o vertex.o window.o upng/upng.o
		

all: $(OBJECTS) Makefile *.c *.h
	ar rcs $(TARGET) $(OBJECTS)

%o: %.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<
clean:
	rm $(OBJECTS) $(TARGET)
