
CFLAGS = -n32

all: res2sdif dotformat2sdif spew-sdif f0tosdif F0tosdif pics2sdif i2sdif


res2sdif: sdif.o res2sdif.o
	cc $(CFLAGS) -o res2sdif sdif.o res2sdif.o -lm

dotformat2sdif: sdif.o dotformat2sdif.o readformat.o
	cc $(CFLAGS) -o dotformat2sdif sdif.o dotformat2sdif.o readformat.o

spew-sdif: sdif.o spew-sdif.o
	cc $(CFLAGS) -o spew-sdif spew-sdif.o sdif.o

f0tosdif: sdif.o f0tosdif.o
	cc $(CFLAGS) -o f0tosdif sdif.o f0tosdif.o

F0tosdif: sdif.o F0tosdif.o
	cc $(CFLAGS) -o F0tosdif sdif.o F0tosdif.o

pics2sdif: sdif.o pics2sdif.o
	cc $(CFLAGS) -o pics2sdif sdif.o pics2sdif.o

i2sdif: sdif.o i2sdif.o
	cc $(CFLAGS) -o i2sdif sdif.o i2sdif.o

sdif.o: sdif.c sdif.h
	cc $(CFLAGS) -c sdif.c

spew-sdif.o: spew-sdif.c sdif.h
	cc $(CFLAGS) -c spew-sdif.c

res2sdif.o: res2sdif.c sdif.h
	cc $(CFLAGS) -c res2sdif.c

f0tosdif.o: f0tosdif.c sdif.h
	cc $(CFLAGS) -c f0tosdif.c

F0tosdif.o: F0tosdif.c sdif.h
	cc $(CFLAGS) -c F0tosdif.c

dotformat2sdif.o: dotformat2sdif.c sdif.h readformat.h
	cc $(CFLAGS) -c dotformat2sdif.c

readformat.o: readformat.c readformat.h
	cc $(CFLAGS) -c readformat.c

pics2sdif.o: pics2sdif.c sdif.h
	cc $(CFLAGS) -c pics2sdif.c

i2sdif.o: i2sdif.c sdif.h
	cc $(CFLAGS) -c i2sdif.c
