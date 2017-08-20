#DFLAGS=
INCLUDE=.
LIBS=-lSDL2 -lGL -lGLEW -lm mrb_lib/mrb_lib.a
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=c99 -pedantic -I$(INCLUDE)
MAKE=make

TARGET=cgame
OBJECTS=main.o game.o 

all: $(OBJECTS) Makefile mrb_lib/mrb_lib.a
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

mrb_lib/mrb_lib.a: mrb_lib/Makefile mrb_lib/*.h mrb_lib/*.c
	$(MAKE) -C mrb_lib

%o: %.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<
clean:
	rm $(OBJECTS) $(TARGET)
	$(MAKE) -C mrb_lib clean
