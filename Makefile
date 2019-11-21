LDFLAGS=-lncurses

all: life.c
	cc life.c $(LDFLAGS) -o life
