CFLAGS = -g -I../lib

# CFLAGS = -n32 -fullwarn -mips3

ALL = TASS


all: $(ALL)


TASS: TASS.o sdif.o sdif-mem.o sdif-types.o sdif-sinusoids.o
	cc $(CFLAGS) -o TASS TASS.o sdif.o sdif-mem.o sdif-types.o sdif-sinusoids.o -lm

sdif.o: ../lib/sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c ../lib/sdif.c

sdif-mem.o: ../lib/sdif-mem.c ../lib/sdif-mem.h
	cc $(CFLAGS) -c ../lib/sdif-mem.c

sdif-types.o: ../lib/sdif-types.c ../lib/sdif-types.h
	cc $(CFLAGS) -c ../lib/sdif-types.c

sdif-sinusoids.o: ../lib/sdif-sinusoids.c ../lib/sdif-sinusoids.h
	cc $(CFLAGS) -c ../lib/sdif-sinusoids.c

clean:
	rm -f *.o core *~ $(ALL)
