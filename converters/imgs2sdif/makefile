CC		= CC
INCDIRS		= -I/usr/local/cast/src/SDIF
CFLAGS		= $(INCDIRS) -fullwarn -n32 -mips3 -LANG:ansi-for-init-scope
OBJS		= /usr/local/cast/lib/sdif.o \
		/usr/local/cast/lib/sdifu.o

all: imgs2sdif

imgs2sdif: imgs2sdif.cxx
	$(CC) $(CFLAGS) -o imgs2sdif imgs2sdif.cxx $(OBJS)

clean:
	rm -f imgs2sdif *~ core
