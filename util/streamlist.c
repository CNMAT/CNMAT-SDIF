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
*/

/* 
    streamlist.c
    stdio utility for listing all the streams in an SDIF file.

    Taken from the MSP SDIF-buffer object 9/27/99 by Matt Wright

*/

#include <stdio.h>
#include "sdif.h"

static void try_file(char *filename);
static void do_streamlist(FILE *f, char *name, int showframes);

int main(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; ++i) {
	try_file(argv[i]);
    }
    return 0;
}

static void try_file(char *filename)  {
    SDIFresult r;
    FILE *f;

    printf("List of streams for SDIF file %s\n", filename);
    r = SDIF_OpenRead(filename, &f);
    if (r) {
	printf("SDIF-buffer: error opening SDIF file %s: %s\n",
	       SDIF_GetErrorString(r), filename);
	return;
    } 

    do_streamlist(f, filename, 0);
    if (r = SDIF_CloseRead(f)) {
	    printf("SDIF-buffer: error closing SDIF file %s:\n", filename);
	    printf("\t%s\n", SDIF_GetErrorString(r));
    }
}

#if 0
void SDIFbuffer_framelist(SDIFBuffer *x, Symbol *fileName) {
    FILE *f;

    printf("SDIFbuffer_framelist for file %s\n", fileName->s_name);
    f = OpenSDIFFile(fileName->s_name);
    if (f == NULL) {
	printf("SDIF-buffer: error %d opening SDIF file %s:", SDIF_GetErrorCode(),
		 fileName->s_name);
	printf("%s\n", SDIF_GetLastErrorString());
	return;
    } 

    do_streamlist(f, fileName->s_name, 1);
    if (SDIF_CloseRead(f) != 0) {
	    printf("SDIF-buffer: error closing SDIF file %s:", fileName->s_name);
	    printf("%s\n", SDIF_GetLastErrorString());
    }

}

#endif


#define MAX_STREAMS 1000  // Most streams any file should have

static void do_streamlist(FILE *f, char *name, int showframes) {
    SDIFresult r;
    SDIF_FrameHeader fh;
    int i;
    
    struct {
        sdif_int32 streamID[MAX_STREAMS];
        char frameType[MAX_STREAMS][4];
        int n;
    } streamsSeen;

    streamsSeen.n = 0;
    
    while ((r = SDIF_ReadFrameHeader(&fh, f)) == 0) {
        for (i = 0; i < streamsSeen.n; ++i) {
            if (streamsSeen.streamID[i] == fh.streamID) {
                /* Already saw this stream, so just make sure type is OK */
                if (!SDIF_Char4Eq(fh.frameType, streamsSeen.frameType[i])) {
                    printf("Warning: First frame for stream %ld\n", fh.streamID);
                    printf(" had type %c%c%c%c, but frame at time %g has type %c%c%c%c\n",
			   streamsSeen.frameType[i][0], streamsSeen.frameType[i][1],
			   streamsSeen.frameType[i][2], streamsSeen.frameType[i][3],
			   fh.time, fh.frameType[0], fh.frameType[1], fh.frameType[2],
			   fh.frameType[3]);
                    }
                    goto skip;
            }
        }
        printf("  <stream id=\"%ld\"/>  frame type %c%c%c%c, starts at time %g\n",
             fh.streamID, fh.frameType[0], fh.frameType[1], fh.frameType[2], fh.frameType[3],
             fh.time);
         
        if (streamsSeen.n >= MAX_STREAMS) {
            printf(" streamlist: error: SDIF file has more than %ld streams!\n", MAX_STREAMS);
        } else {
            streamsSeen.streamID[streamsSeen.n] = fh.streamID;
            SDIF_Copy4Bytes(streamsSeen.frameType[streamsSeen.n], fh.frameType);
            ++(streamsSeen.n);
        }
        
        skip:         

        if (showframes) {
            printf("    Frame type %c%c%c%c, size %d, time %f, StreamID %d, %d matrices\n",
                     fh.frameType[0], fh.frameType[1], fh.frameType[2], fh.frameType[3],
                     fh.size, (float) fh.time, fh.streamID, fh.matrixCount);
        }

        if (r = SDIF_SkipFrame(&fh, f)) {
            printf("SDIF-buffer: error skipping frame in SDIF file %s:", name);
            printf("%s\n", SDIF_GetErrorString(r));
	    return;
        }
    }

    if (r != ESDIF_END_OF_DATA) {
        printf("SDIF-buffer: error reading SDIF file %s:", name);
        printf("%s\n", SDIF_GetErrorString(r));
    }
}
