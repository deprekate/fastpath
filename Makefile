IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

SDIR=./src/

LIBS=-lm

all: fastpath fastpathz

fastpath: $(SDIR)fastpath.c
	gcc -o $@ $^ $(CFLAGS)
fastpathz: $(SDIR)fastpathz.c $(SDIR)mini-gmp.c
	gcc -o $@ $^ $(CFLAGS) -g
