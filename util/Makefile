CFLAGS = -I../lib

ALL =  spew-sdif merge-sdif streamlist copy-sdif change-streamID

all: $(ALL)

merge-sdif: merge-sdif.o sdif.o 
	cc $(CFLAGS) -o merge-sdif merge-sdif.o sdif.o 

copy-sdif: copy-sdif.o sdif.o
	cc $(CFLAGS) -o copy-sdif copy-sdif.o sdif.o

streamlist: streamlist.o sdif.o
	cc $(CFLAGS) -o  streamlist streamlist.o sdif.o

spew-sdif: sdif.o spew-sdif.o
	cc $(CFLAGS) -o spew-sdif spew-sdif.o sdif.o

change-streamID: sdif.o change-streamID.o
	cc $(CFLAGS) -o change-streamID change-streamID.o sdif.o

sdif.o: ../lib/sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c ../lib/sdif.c

spew-sdif.o: spew-sdif.c ../lib/sdif.h
	cc $(CFLAGS) -c spew-sdif.c

clean:
	rm -f *.o core *~ $(ALL)

