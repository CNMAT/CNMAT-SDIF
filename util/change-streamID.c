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

  change-streamID.c

  Change the ID(s) of one or more SDIF streams in a file.

  Matt Wright, 6/5/2000

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "sdif.h"

void DoCopy(FILE *inf, FILE *outf, int oldStreamNum, int newStreamNum) {
    SDIF_FrameHeader fh;
    SDIF_MatrixHeader mh;
    SDIFresult r;
    void *buffer;
    int size;

    int i;
    

    while ((r = SDIF_ReadFrameHeader(&fh, inf)) == ESDIF_SUCCESS) {
	/* Here's where you'd look at fh.frameType and/or
	   fh.streamID to decide what to do with the frame */
	    if(fh.streamID == oldStreamNum){
		    fh.streamID = newStreamNum;
	    }

	r = SDIF_WriteFrameHeader(&fh, outf);
	if (r) {
	    fprintf(stderr, "Error writing frame header: %s\n", 
		    SDIF_GetErrorString(r));
	    return;
	}

	//printf("* %d matrices\n", fh.matrixCount);
	for (i = 0; i < fh.matrixCount; ++i) {
	    r=SDIF_ReadMatrixHeader(&mh, inf);
	    if (r) {
		fprintf(stderr, "Error reading matrix header: %s\n",
			SDIF_GetErrorString(r));
		return;
	    }

	    r=SDIF_WriteMatrixHeader(&mh, outf);
            if (r) {
                fprintf(stderr, "Error writing matrix header: %s\n",
                        SDIF_GetErrorString(r));
                return;
	    }

	    size=SDIF_GetMatrixDataSize(&mh);
	    //printf("* Matrix size %d\n", size);
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

void CopySDIF(char *inFileName, char *oldStreamNum, char *newStreamNum) {
    FILE *inf, *outf;
    SDIFresult r;
    char outFileName[256];
    struct timeval tv;
    int os, ns;
    os = atoi(oldStreamNum);
    ns = atoi(newStreamNum);
    gettimeofday(&tv, NULL);
    sprintf(outFileName, "%s.tmp%d", inFileName, tv.tv_usec);
    //printf("%s %s %d %d", inFileName, outFileName, os, ns);

    //printf("CopySDIF %s %s\n", inFileName, outFileName);

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

    DoCopy(inf, outf, os, ns);

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

    if(rename(outFileName, inFileName)){
	    fprintf(stderr, "couldn't rename tmp file\n");
    }

}

int main(int argc, char *argv[]) {


    if (argc != 4) {
	/* goto usage; */

	fprintf(stderr, "Usage: %s input.sdif oldStreamNumber newStreamNumber\n", argv[0]);
	exit(1);
    }

    CopySDIF(argv[1], argv[2], argv[3]);
    return 0;
}
