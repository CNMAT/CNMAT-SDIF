#include <stdio.h>
#include <stdlib.h>
#include "sdif.h"
#include "sdif-mem.h"


void *my_malloc(int numBytes) {
    return (void *) malloc(numBytes);
}

void my_free(void *memory, int numBytes) {
    free(memory);
}


void Test(char *infilename, char *outfilename) {
    FILE *inf;
    FILE *outf;
    SDIFmem_Frame f;
    SDIFmem_Matrix m, m2;
    int i;
    sdif_float32 floats[40];
    sdif_int32 ints[40];


    inf = SDIF_OpenRead(infilename);
    if (inf == NULL) {
	printf("Couldn't open to read: %s\n", SDIF_GetLastErrorString());
	return;
    }

    outf = SDIF_OpenWrite(outfilename);
    if (outf == NULL) {
	printf("Couldn't open to write: %s\n", SDIF_GetLastErrorString());
	return;
    }

    /* First frame: no matrices */
    f = SDIFmem_CreateEmptyFrame();
    SDIF_Copy4Bytes(f->header.frameType, "XFOO");
    f->header.time = -999;
    f->header.streamID = -888;
    f->header.size = 16;
    f->header.matrixCount = 0;

    SDIFmem_WriteFrame(outf, f);


    /* Second frame: one empty matrix */

    f->header.time = -888;
    m = SDIFmem_CreateEmptyMatrix();
    f->matrices = m;

    SDIF_Copy4Bytes(m->header.matrixType, "XBAR");
    m->header.matrixDataType = SDIF_UTF8;
    m->header.rowCount = m->header.columnCount = 0;
    SDIFmem_RepairFrameHeader(f);
    SDIFmem_WriteFrame(outf, f);



    /* Third frame: data in the matrix */
    f->header.time = -777;
    f->header.size = 16 + sizeof(SDIF_MatrixHeader) + 40 *sizeof(sdif_float32);
    m->header.matrixDataType = SDIF_FLOAT32;
    m->header.rowCount = 10;
    m->header.columnCount = 4;

    for (i = 0; i < 40; ++i) {
	floats[i] = (float) (i * 100.0f);
	ints[i] = i * 10;
    }
    m->data = floats;
    SDIFmem_WriteFrame(outf, f);


    /* Fourth frame: ints */
    f->header.time = -666;

    m2 = SDIFmem_CreateEmptyMatrix();
    SDIF_Copy4Bytes(m2->header.matrixType, "XBAZ");
    m2->header.matrixDataType = SDIF_INT32;
    m2->header.rowCount = 8;
    m2->header.columnCount = 5;
    m2->data = ints;
    
    if (SDIFmem_AddMatrix(f, m2) != 1) {
	printf("SDIFmem_AddMatrix isn't happy.\n");
	return;
    }

    SDIFmem_WriteFrame(outf, f);

    /* Test text */
    f->header.time = -555;
    m->header.matrixDataType = SDIF_UTF8;
    m->header.rowCount = 12;
    m->header.columnCount = 1;
    m->data = "matt + erika";
    SDIFmem_RepairFrameHeader(f);
    SDIFmem_WriteFrame(outf, f);
    

    SDIFmem_FreeMatrix(m);
    SDIFmem_FreeMatrix(m2);
    SDIFmem_FreeFrame(f);

    /* And then copy the first frame from the input file */

    f = SDIFmem_ReadFrame(inf, 0, 0);
    if (f == 0) {
        printf("SDIFmem_ReadFrame isn't happy.\n");
        return;
    }
    SDIFmem_WriteFrame(outf, f);

}


void main(int argc, char **argv) {
    if (argc != 3) {
	printf("Test in.sdif out.sdif\n");
	return;
    }

    SDIFmem_Init(my_malloc, my_free);

    Test(argv[1], argv[2]);
}
