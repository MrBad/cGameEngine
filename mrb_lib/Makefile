DFLAGS=-DCOMPILE_TESTS
INCLUDE=-I.
LIBS=-lSDL2 -lGL -lGLEW -lm
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=c99 -pedantic $(INCLUDE) $(DFLAGS)
MAKE=make

TARGET=mrb_lib.a
OBJECTS=camera.o file_get.o gl_program.o inmgr.o mat4f.o \
		sprite.o sprite_batch.o texture.o vertex.o window.o \
		array.o \
		upng/upng.o
		

all: $(OBJECTS) Makefile *.c *.h
	ar rcs $(TARGET) $(OBJECTS)

upng/upng.o: upng/upng.c upng/upng.h
	$(CC) $(CFLAGS) -Wno-unused-but-set-variable $(OFLAGS) -o $@ $<

%o: %.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<

clean:
	rm $(OBJECTS) $(TARGET)
