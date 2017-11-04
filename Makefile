DESTDIR ?= $(HOME)
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

all:
	g++ -std=c++14 main.cpp -lncurses -O3 -o fireplace

install:
	install -d $(DESTDIR)/$(BINDIR)
	install -m 755 fireplace $(DESTDIR)/$(BINDIR)

uninstall:
	rm $(DESTDIR)/$(BINDIR)/fireplace
