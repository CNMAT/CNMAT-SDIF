/*
Copyright (c) 2000.  The Regents of the University of California (Regents).
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

  text2sdif.c

  Turn a text file into an SDIF file

  Matt Wright, 6/5/2000
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "sdif.h"


void DoWrite(FILE *inf, FILE *outf) {
    SDIF_FrameHeader fh;
    SDIF_MatrixHeader mh;
    SDIFresult r;
    long size;
    int i, c;
    char c2;

    /* How much text is there? */

    if (fseek(inf, 0, SEEK_END)) {
	fprintf(stderr, "fseek failed finding size of text file: %s\n",
		strerror(errno));
    }

    size = ftell(inf);
    if (size == -1L) {
	fprintf(stderr, "ftell failed finding size of text file: %s\n",
		strerror(errno));
    }

    rewind(inf);

    /* printf("Your file is %d bytes.\n", size); */

    
    /* Write headers for SDIF file */
    
    SDIF_Copy4Bytes(fh.frameType, "XTXT");
    fh.time = -HUGE_VAL;
    fh.streamID = 1000;
    fh.matrixCount = 1;

    SDIF_Copy4Bytes(mh.matrixType, "XTXT");
    mh.matrixDataType=SDIF_UTF8;
    mh.rowCount = size+1;  /* including null byte */
    mh.columnCount = 1;

    fh.size = 16 + sizeof(mh) + SDIF_GetMatrixDataSize(&mh);

    r = SDIF_WriteFrameHeader(&fh, outf);
    if (r) {
	fprintf(stderr, "Error writing frame header: %s\n", 
		SDIF_GetErrorString(r));
	return;
    }

    r=SDIF_WriteMatrixHeader(&mh, outf);
    if (r) {
	fprintf(stderr, "Error writing matrix header: %s\n",
		SDIF_GetErrorString(r));
	return;
    }

    for (i=0; i<size; ++i) {
	c=fgetc(inf);
	if (c==EOF) {
	    fprintf(stderr, "Expected %d characters in text file, but EOF after reading %d\n",
		    size, i);
	    return;
	}

#define IsASCII(c) (!((c) & 128))
	if (!IsASCII(c)) {
	    fprintf(stderr, "Non-ASCII character %d in text file; bombing out\n", c);
	    return;
	}

	c2 = (char) c;
	r=SDIF_Write1(&c2, 1, outf);

	if (r) {
	    fprintf(stderr, "Error writing byte to SDIF file: %s\n",
		    SDIF_GetErrorString(r));
	    return;
	}
    }

    /* Now the null byte */
    c2 = 0;
    r=SDIF_Write1(&c2, 1, outf);

    if (r) {
	fprintf(stderr, "Error writing null byte to SDIF file: %s\n",
		SDIF_GetErrorString(r));
	return;
    }

    /* Now any possible padding */
    SDIF_WriteMatrixPadding(outf, &mh);
}

#if 0

void DoCopy(

    

    while ((r = SDIF_ReadFrameHeader(&fh, inf)) == ESDIF_SUCCESS) {
	/* Here's where you'd look at fh.frameType and/or
	   fh.streamID to decide what to do with the frame */

	printf("* %d matrices\n", fh.matrixCount);
	for (i = 0; i < fh.matrixCount; ++i) {
	    r=SDIF_ReadMatrixHeader(&mh, inf);
	    if (r) {
		fprintf(stderr, "Error reading matrix header: %s\n",
			SDIF_GetErrorString(r));
		return;
	    }


	    size=SDIF_GetMatrixDataSize(&mh);
	    printf("* Matrix size %d\n", size);
	    buffer = malloc(size);
	    if (!buffer) {
		fprintf(stderr, "Out of memory\n");
		return;
	    }

	    r=SDIF_ReadMatrixData(buffer, inf, &mh);
            if (r) {
                fprintf(stderr, "Error reading matrix data: %s\n",
                        SDIF_GetErrorString(r));
                return;
	    }

	    r=SDIF_WriteMatrixData(outf, &mh, buffer);
            if (r) {
                fprintf(stderr, "Error writing matrix data: %s\n",
                        SDIF_GetErrorString(r));
                return;
	    }

	    free(buffer);
	}
    }

    if (r != ESDIF_END_OF_DATA) {
	fprintf(stderr, "Error reading SDIF data: %s\n", 
		SDIF_GetErrorString(r));
    }
}

#endif

void TextToSDIF(char *inFileName, char *outFileName) {
    FILE *inf, *outf;
    SDIFresult r;


    inf=fopen(inFileName, "r");

    if (inf==NULL) {
	fprintf(stderr, "Couldn't open %s for reading: %s\n",
		inFileName, strerror(errno));
	return;
    }

    r = SDIF_OpenWrite(outFileName, &outf);
    if (r) {
	fprintf(stderr, "Couldn't open %s for writing: %s\n",
		outFileName, SDIF_GetErrorString(r));
	return;
    }

    DoWrite(inf, outf);

    r = SDIF_CloseWrite(outf);
    if (r) {
        fprintf(stderr, "Couldn't close file %s after writing: %s\n",
                outFileName, SDIF_GetErrorString(r));
        return;
    }

    fclose(inf);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
	fprintf(stderr, "Usage: %s textfile output.sdif\n", argv[0]);
	return -1;
    }

    TextToSDIF(argv[1], argv[2]);
    return 0;
}
