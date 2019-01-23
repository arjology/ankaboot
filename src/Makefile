BDIR=../build
IDIR=../include
TDIR=../test
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS= hash_table.h xmalloc.h prime.h graph_elements.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o hash_table.o xmalloc.o prime.o graph_elements.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: $(OBJ)
	${CC} -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

build-test: clean
	${CC} ${CFLAGS} -o $(BDIR)/hash_table_test hash_table.c $(TDIR)/hash_table_test.c xmalloc.c prime.c

.PHONY: clean

clean:
	rm -rf $(BDIR)
	mkdir -p $(BDIR)
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

test: build-test
	$(BDIR)/hash_table_test
