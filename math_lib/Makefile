#DFLAGS=
INCLUDE=.
LIBS=-lSDL2 -lGL -lGLEW -lm
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=c99 -pedantic -I$(INCLUDE)
MAKE=make

TARGET=math_lib.a
OBJECTS=mat4f.o

all: $(OBJECTS) Makefile
	ar rcs math_lib.a $(OBJECTS)

%o: %.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<
clean:
	rm $(OBJECTS) $(TARGET)
