CC		= CC
INCDIRS		= -I/usr/local/sdif/src
CFLAGS		= $(INCDIRS) -fullwarn -n32 -mips3 -LANG:ansi-for-init-scope
LIBS		= -laudiofile
OBJS		= sdif2sf.o \
		soundfiles.o \
		$(SDIF_LIB)/sdif.o \
		$(SDIF_LIB)/sdifu.o

all: sdif2sf

sdif2sf: $(OBJS)
	$(CC) $(CFLAGS) -o sdif2sf $(OBJS) $(LIBS)

sdif2sf.o: sdif2sf.cxx
	$(CC) $(CFLAGS) -c sdif2sf.cxx

clean:
	rm -f sdif2sf sdif2sf.o *~ core
