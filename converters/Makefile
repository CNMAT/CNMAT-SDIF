CFLAGS = -I../lib

ALL = dotformat2sdif f0tosdif F0tosdif pics2sdif i2sdif res2sdif

all: $(ALL)

dotformat2sdif: sdif.o dotformat2sdif.o readformat.o sdif-types.o
	cc $(CFLAGS) -o dotformat2sdif sdif.o dotformat2sdif.o readformat.o sdif-types.o -lm

f0tosdif: sdif.o f0tosdif.o
	cc $(CFLAGS) -o f0tosdif sdif.o f0tosdif.o

F0tosdif: sdif.o F0tosdif.o
	cc $(CFLAGS) -o F0tosdif sdif.o F0tosdif.o

pics2sdif: sdif.o pics2sdif.o
	cc $(CFLAGS) -o pics2sdif sdif.o pics2sdif.o

i2sdif: sdif.o i2sdif.o
	cc $(CFLAGS) -o i2sdif sdif.o i2sdif.o

res2sdif: sdif.o res2sdif.o
	cc $(CFLAGS) -o res2sdif sdif.o res2sdif.o

sdif.o: ../lib/sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c ../lib/sdif.c

sdif-types.o: ../lib/sdif-types.c ../lib/sdif-types.h
	cc $(CFLAGS) -c ../lib/sdif-types.c


spew-sdif.o: spew-sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c spew-sdif.c

f0tosdif.o: f0tosdif.c ../lib/sdif.h
	cc $(CFLAGS) -c f0tosdif.c

F0tosdif.o: F0tosdif.c ../lib/sdif.h
	cc $(CFLAGS) -c F0tosdif.c

dotformat2sdif.o: dotformat2sdif.c ../lib/sdif.h readformat.h
	cc $(CFLAGS) -c dotformat2sdif.c

readformat.o: readformat.c readformat.h
	cc $(CFLAGS) -c readformat.c

pics2sdif.o: pics2sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c pics2sdif.c

clean:
	rm -f *.o core *~ $(ALL)

