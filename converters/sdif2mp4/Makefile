CC = cc
CXX = CC

ENDIAN = -DBIG_ENDIAN -D__BYTE_INCOHERENT
# ENDIAN = -DLITTLE_ENDIAN -D__BYTE_COHERENT

CFLAGS = -g -n32 -mips3 -INLINE:NO -c $(ENDIAN)
LDFLAGS = -mips3 -n32 -lmalloc_cv

SOURCE = bitstream.cpp sa_encode.yy.c saol_tok_table.cpp sdif.c sdif2mp4.cpp\
	sdif.h bitstream.h port.h sa_bitstream.h saol.tab.h saol_tok_table.h\
	Makefile

OBJS =  bitstream.o sa_encode.yy.o saol_tok_table.o sdif.o sdif2mp4.o

all: sdif2mp4
 
sdif2mp4: $(OBJS) Makefile
	$(CXX) $(OBJS) -o sdif2mp4 $(LDFLAGS)

public: $(SOURCE)
	tar -cvLf sdif2mp4.tar $(SOURCE)
	gzip sdif2mp4.tar

clean:
	/bin/rm -f sdif2mp4 *o

bitstream.o: bitstream.cpp port.h bitstream.h
	$(CXX) $(CFLAGS) bitstream.cpp 

sa_encode.yy.o: sa_encode.yy.c saol.tab.h
	$(CC) $(CFLAGS) -w sa_encode.yy.c

saol_tok_table.o: saol_tok_table.cpp saol.tab.h saol_tok_table.h
	$(CXX) $(CFLAGS) saol_tok_table.cpp

sdif.o: sdif.c sdif.h
	$(CC) $(CFLAGS) sdif.c

sdif2mp4.o: sdif2mp4.cpp bitstream.h sa_bitstream.h saol_tok_table.h sdif.h saol.tab.h
	$(CXX) $(CFLAGS) sdif2mp4.cpp

