CC = gcc
CFLAGS =-c
LNFLAGS = -lncurses
DESTDIR ?= $(HOME)/bin
EXEC = fireplace

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

all: $(EXEC)

.PHONY: debug clean install uninstall

.PHONY: notcurses
notcurses: LNFLAGS=-L/usr/local/lib -lnotcurses 
notcurses: CFLAGS+= -I/usr/local/include -DNOTCURSES
notcurses: $(EXEC)
	echo "wowee"

debug: CFLAGS=-c -g -gdwarf-2 -g3
debug: $(EXEC)
	echo "oops"

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(LNFLAGS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(EXEC) *.o

install:
	install -d $(DESTDIR)
	install -m 755 $(EXEC) $(DESTDIR)

uninstall:
	rm $(DESTDIR)/$(EXEC)

