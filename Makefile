CC = gcc
CFLAGS = -O3
LNFLAGS = -lncurses
DESTDIR ?= $(HOME)/bin
EXEC = fireplace

all:
	$(CC) main.c $(CFLAGS) $(LNFLAGS) -o $(EXEC)

install:
	install -d $(DESTDIR)
	install -m 755 $(EXEC) $(DESTDIR)

uninstall:
	rm $(DESTDIR)/$(EXEC)
