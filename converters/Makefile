CFLAGS = -I../lib -I../../lib/soundfiles/

ALL = dotformat2sdif f0tosdif capital-F0tosdif pics2sdif res2sdif text2sdif \
	  sdif2dotformat
# i2sdif sdif2sf sf2sdif

all: $(ALL)

dotformat2sdif: sdif.o dotformat2sdif.o readformat.o sdif-types.o
	cc $(CFLAGS) -o dotformat2sdif sdif.o dotformat2sdif.o readformat.o sdif-types.o -lm

f0tosdif: sdif.o f0tosdif.o
	cc $(CFLAGS) -o f0tosdif sdif.o f0tosdif.o

capital-F0tosdif: sdif.o capital-F0tosdif.o
	cc $(CFLAGS) -o capital-F0tosdif sdif.o capital-F0tosdif.o

pics2sdif: sdif.o pics2sdif.o
	cc $(CFLAGS) -o pics2sdif sdif.o pics2sdif.o

i2sdif: sdif.o i2sdif.o
	cc $(CFLAGS) -o i2sdif sdif.o i2sdif.o

res2sdif: sdif.o res2sdif.o sdif-types.o sdif-mem.o
	cc $(CFLAGS) -o res2sdif sdif.o res2sdif.o sdif-mem.o sdif-types.o -lm

text2sdif: sdif.o text2sdif.o
	cc $(CFLAGS) -o text2sdif sdif.o text2sdif.o

sdif2sf: sdif2sf.o sdif.o soundfiles.o
	cc $(CFLAGS) -o sdif2sf sdif2sf.o sdif.o soundfiles.o -laudiofile

sf2sdif: sf2sdif.o sdif.o soundfiles.o
	cc $(CFLAGS) -o sdif2sf sf2sdif.o sdif.o soundfiles.o -laudiofile

sdif2dotformat: sdif.o sdif2dotformat.o
	cc $(CFLAGS) -o sdif2dotformat sdif2dotformat.o sdif.o

soundfiles.o: ../../lib/soundfiles/soundfiles.c ../../lib/soundfiles/soundfiles.h
	cc $(CFLAGS) -c ../../lib/soundfiles/soundfiles.c

sdif.o: ../lib/sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c ../lib/sdif.c

sdif-mem.o: ../lib/sdif-mem.c ../lib/sdif-mem.h
	cc $(CFLAGS) -c ../lib/sdif-mem.c

sdif-types.o: ../lib/sdif-types.c ../lib/sdif-types.h
	cc $(CFLAGS) -c ../lib/sdif-types.c


f0tosdif.o: f0tosdif.c ../lib/sdif.h
	cc $(CFLAGS) -c f0tosdif.c

capital-F0tosdif.o: capital-F0tosdif.c ../lib/sdif.h
	cc $(CFLAGS) -c capital-F0tosdif.c

dotformat2sdif.o: dotformat2sdif.c ../lib/sdif.h readformat.h
	cc $(CFLAGS) -c dotformat2sdif.c

readformat.o: readformat.c readformat.h
	cc $(CFLAGS) -c readformat.c

pics2sdif.o: pics2sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c pics2sdif.c

res2sdif.o: res2sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c res2sdif.c

i2sdif.o: i2sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c i2sdif.c



clean:
	rm -f *.o core *~ $(ALL)

