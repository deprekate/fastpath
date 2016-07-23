IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =./lib

LIBS=-lm

_DEPS = uthash.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = fastpath.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

fastpath: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
