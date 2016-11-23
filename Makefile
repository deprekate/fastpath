IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =./lib

LIBS=-lm
LIBS_I=-lgmp

_DEPS = uthash.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_DEPS_I = uthash.h gmp.h
DEPS_I = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = fastpath.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_I = fastpathz.o
OBJ_I = $(patsubst %,$(ODIR)/%,$(_OBJ_I))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


all: fastpath fastpathz

fastpath: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)
fastpathz: $(OBJ_I)
	gcc -o $@ $^ $(CFLAGS) $(LIBS_I)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
