SRCDIR = src
HEADDIR = include
LIBDIR = ./src

DEBUGFLAGS = -W -Wall -Wextra -Wvla -fsanitize=address
FLAGS = 
DEPENDENCIES = 

build:
	gcc $(SRCDIR)/raycaster.c -o ./bin/raycaster $(DEPENDENCIES) $(FLAGS) 

run:
	./bin/raycaster

clean:
	rm ./bin/raycaster

debug:
	gcc $(SRCDIR)/raycaster.c -o ./bin/raycaster $(DEPENDENCIES) $(FLAGS) $(DEBUGFLAGS) 

all:
	make build
	make run

install:
	apt install libglew-dev libsdl2-dev freeglut3-dev