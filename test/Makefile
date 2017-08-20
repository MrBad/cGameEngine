INCLUDE=../math_lib
LIBS=-lm
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=c99 -pedantic --pedantic-errors -I$(INCLUDE)

TARGET=test

OBJS=test.o camera.o 

all: $(OBJS) Makefile ../math_lib/*.h
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%o: %c Makefile *.h ../math_lib/*.h
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<

clean:
	rm $(OBJS) $(TARGET)

