/*
Copyright (c) 1999.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Matt Wright, The Center for New Music and Audio Technologies,
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
     ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
     REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.


  test-sdif-mem.c:  put sdif-mem.c through its paces

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* For strlen() */
#include "sdif.h"
#include "sdif-mem.h"


void *my_malloc(int numBytes) {
    return (void *) malloc(numBytes);
}

void my_free(void *memory, int numBytes) {
    free(memory);
}


void Test(char *infilename, char *outfilename) {
    SDIFresult r;
    FILE *inf;
    FILE *outf;
    SDIFmem_Frame f;
    SDIFmem_Matrix m, m2;
    int i;
    sdif_float32 floats[40];
    sdif_int32 ints[40];


    r = SDIF_OpenRead(infilename, &inf);
    if (r) {
	printf("Couldn't open to read: %s\n", SDIF_GetErrorString(r));
	return;
    }

    r = SDIF_OpenWrite(outfilename, &outf);
    if (r) {
	printf("Couldn't open to write: %s\n", SDIF_GetErrorString(r));
	return;
    }

    /* First frame: no matrices */
    f = SDIFmem_CreateEmptyFrame();
    if (!f) {
        printf("Couldn't create empty frame.  Out of memory?\n");
        return;
    }

    SDIF_Copy4Bytes(f->header.frameType, "XFOO");
    f->header.time = -999;
    f->header.streamID = -888;
    f->header.size = 16;
    f->header.matrixCount = 0;

    if (r = SDIFmem_WriteFrame(outf, f)) {
        printf("SDIFmem_WriteFrame isn't happy: %s\n", SDIF_GetErrorString(r));
        return;
    }

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


    /* Fourth frame: ints, and padding required. */
    f->header.time = -666;

    m2 = SDIFmem_CreateEmptyMatrix();
    SDIF_Copy4Bytes(m2->header.matrixType, "XBAZ");
    m2->header.matrixDataType = SDIF_INT32;
    m2->header.rowCount = 13;
    m2->header.columnCount = 3;
    m2->data = ints;
    
    if (r = SDIFmem_AddMatrix(f, m2)) {
	printf("SDIFmem_AddMatrix isn't happy: %s\n", SDIF_GetErrorString(r));
	return;
    }

    SDIFmem_WriteFrame(outf, f);

    SDIFmem_FreeMatrix(m2);
    m->next = 0;

    /* Test text */
    f->header.time = -555;
    SDIF_Copy4Bytes(m->header.matrixType, "XTXT");
    m->header.matrixDataType = SDIF_UTF8;
    m->data = "SDIF is cool";
    m->header.rowCount = strlen(m->data) + 1;
    m->header.columnCount = 1;
    SDIFmem_RepairFrameHeader(f);
    SDIFmem_WriteFrame(outf, f);
    

    SDIFmem_FreeMatrix(m);
    SDIFmem_FreeFrame(f);


#ifdef COPY_ALL_FRAMES
    /* And then all frames from the input file */

    while((r = SDIFmem_ReadFrame(inf, &f)) != ESDIF_END_OF_DATA) {
	if (r) {
	    printf("SDIFmem_ReadFrame isn't happy: %s\n", SDIF_GetErrorString(r));
	    return;
	}
	SDIFmem_WriteFrame(outf, f);
    }
#else
    /* And then copy the first frame from the input file */

    r = SDIFmem_ReadFrame(inf, &f);
    if (r) {
        printf("SDIFmem_ReadFrame isn't happy: %s\n", SDIF_GetErrorString(r));
        return;
    }
    SDIFmem_WriteFrame(outf, f);
#endif

}


void main(int argc, char **argv) {
    if (argc != 3) {
	printf("Test in.sdif out.sdif\n");
	return;
    }

    SDIFmem_Init(my_malloc, my_free);

    Test(argv[1], argv[2]);
}
