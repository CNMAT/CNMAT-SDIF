CFLAGS =

ALL_O= sdif-buf.o sdif-interp-implem.o sdif-interp.o sdif-mem.o sdif-sinusoids.o sdif-types.o sdif-util.o sdif.o 

DEPRECATED_O = sdifu.o

all:  $(ALL_O)
	$(CC) $(CFLAGS) $(ALL_O)

test-sdif-mem: test-sdif-mem.o sdif.o sdif-mem.o
	$(CC) $(CFLAGS) -o test-sdif-mem test-sdif-mem.o sdif.o sdif-mem.o

clean:
	rm -f *.o core *~ test-sdif-mem








