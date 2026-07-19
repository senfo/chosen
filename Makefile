CC = clang

CFLAGS = -Wall -Wextra $(shell pkg-config --cflags raylib)
LDFLAGS = $(shell pkg-config --libs raylib)

.PHONY: all clean sprite scroll

all: sprite

game: main.c
	$(CC) main.c -o game $(CFLAGS) $(LDFLAGS)

sprite: sprite.c
	$(CC) sprite.c -o sprite $(CFLAGS) $(LDFLAGS)

scroll: scroll.c
	$(CC) scroll.c -o scroll $(CFLAGS) $(LDFLAGS)

clean:
	rm -f game sprite scroll
