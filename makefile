CFLAGS=-O3 -funroll-loops -m64 -I.
#CFLAGS=  -g3 -m64 -I.

all: bitmat

bitmat: main.c bitmatops.c graphops.c miscops.c bitmat.h
	g++ $(CFLAGS) -o bin/bitmat bitmatops.c graphops.c miscops.c prune_triples_sim.c main.c

clean:
	rm bin/bitmat
