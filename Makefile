CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)

all: tracker-gui

tracker-gui: gui.c
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) gui.c -o tracker-gui $(RAYLIB_LIBS) -lm

clean:
	rm -f tracker-gui tracker-gui.exe

.PHONY: all clean
