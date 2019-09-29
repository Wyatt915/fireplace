DESTDIR ?= $(HOME)/bin
EXEC = fireplace

all:
	g++ -std=c++14 main.cpp -lncurses -O3 -o $(EXEC)

install:
	install -d $(DESTDIR)
	install -m 755 $(EXEC) $(DESTDIR)

uninstall:
	rm $(DESTDIR)/$(EXEC)
