LDFLAGS = -lncurses
CFLAGS  = -fsanitize=undefined

all: life.c
	cc life.c $(LDFLAGS) $(CFLAGS) -o life
