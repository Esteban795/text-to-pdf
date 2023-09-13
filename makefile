SRCDIR = src
HEADDIR = include
LIBDIR = ./src

DEBUGFLAGS = -W -Wall -Wextra -Wvla -fsanitize=address -g

build:
	gcc $(SRCDIR)/text-to-pdf.c -o ./bin/text-to-pdf

run:
	./bin/text-to-pdf

clean:
	rm ./bin/text-to-pdf

debug:
	gcc $(SRCDIR)/text-to-pdf.c -o ./bin/text-to-pdf $(DEBUGFLAGS) 

all:
	make build
	make run