/*
Copyright (c) 2001.  The Regents of the University of California (Regents).
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for educational, research, and not-for-profit purposes, without
fee and without a signed licensing agreement, is hereby granted, provided that
the above copyright notice, this paragraph and the following two paragraphs
appear in all copies, modifications, and distributions.  Contact The Office of
Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.

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

  fix-frame-size.c

  Correct the frame size field for all frames in an SDIF file

   Matt Wright, 11/30/2001

*/

#include <stdio.h>
#include <stdlib.h>
#include "sdif.h"
#include "sdif-mem.h"

/* Prototypes */
void *MyMalloc(int numBytes);
void MyFree(void *memory, int numBytes);


void *MyMalloc(int numBytes) {
    void * r = malloc(numBytes);
/*    printf("MyMalloc(%d): %p\n", numBytes, r); */
    return r;
}

void MyFree(void *memory, int numBytes) {
/*     printf("MyFree(%p, %d)\n", memory, numBytes); */
    free(memory);
}


void DoFixFrameSize(FILE *inf, FILE *outf) {
    SDIF_FrameHeader fh;
    SDIFresult r;
    void *buffer;
    int size;
    SDIFmem_Frame mframe;

    int i;
    
    while ((r = SDIF_ReadFrameHeader(&fh, inf)) == ESDIF_SUCCESS) {
	
	r=SDIFmem_ReadFrameContents(&fh, inf, &mframe);
	if (r) {
	    fprintf(stderr, "Error %d reading frame contents: %s\n",
		    r, SDIF_GetErrorString(r));
	    return;
	}

	SDIFmem_RepairFrameHeader(mframe);

	r = SDIFmem_WriteFrame(outf, mframe);
	if (r) {
	    fprintf(stderr, "Error writing frame: %s\n", 
		    SDIF_GetErrorString(r));
	    return;
	}
    }

    if (r != ESDIF_END_OF_DATA) {
	fprintf(stderr, "Error reading SDIF data: %s\n", 
		SDIF_GetErrorString(r));
    }
}

void FixSDIF(char *inFileName, char *outFileName) {
    FILE *inf, *outf;
    SDIFresult r;

    printf("Fixing frame size of %s to make %s\n", inFileName, outFileName);

    r = SDIF_OpenRead(inFileName, &inf);
    if (r) {
	fprintf(stderr, "Couldn't open %s for reading: %s\n",
		inFileName, SDIF_GetErrorString(r));
	return;
    }

    r = SDIF_OpenWrite(outFileName, &outf);
    if (r) {
	fprintf(stderr, "Couldn't open %s for writing: %s\n",
		outFileName, SDIF_GetErrorString(r));
	return;
    }

    DoFixFrameSize(inf, outf);

    r = SDIF_CloseWrite(outf);
    if (r) {
        fprintf(stderr, "Couldn't close file %s after writing: %s\n",
                outFileName, SDIF_GetErrorString(r));
        return;
    }

    r = SDIF_CloseRead(inf);
    if (r) {
        fprintf(stderr, "Couldn't close file %s after reading: %s\n",
                inFileName, SDIF_GetErrorString(r));
        return;
    }
}

int main(int argc, char *argv[]) {
    SDIFresult r;

    if (argc != 3) {
	fprintf(stderr, "Usage: %s input.sdif output.sdif\n", argv[0]);
	exit(1);
    }

    if (r = SDIF_Init()) {
	fprintf(stderr, "Error initializing SDIF: %s\n", 
		SDIF_GetErrorString(r));
	return -3;
    }

    if (r = SDIFmem_Init(MyMalloc, MyFree)) {
	fprintf(stderr, "Error initializing SDIFmem: %s\n", 
		SDIF_GetErrorString(r));
	return -4;
    }

    FixSDIF(argv[1], argv[2]);
    return 0;
}
