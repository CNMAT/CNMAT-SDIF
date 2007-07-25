/* 

Copyright (c) 2001.  The Regents of the University of California
(Regents).  All Rights Reserved.

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

 sdif2dotformat.c

 I should be ashamed of myself for writing this.

 Matt Wright, 5/27/2001

*/

#include <stdio.h>
#include <stdlib.h>
#include "sdif.h"
#include "string.h"

void SDIF2DF(char *filename, sdif_int32 streamID);

int main(int argc, char *argv[]) {
  int i;

  if (argc < 3) {
    fprintf(stderr, "Usage: \n");
    fprintf(stderr, "   %s foo.sdif streamID [bar.sdif stream ID...] \n",
	    argv[0]);
    return 1;
  }

  /* Parse args */
  for (i = 1; i < argc-1; i += 2) {
    fprintf(stderr, "Trying file %s stream ID %s\n", argv[i], argv[i+1]);

    SDIF2DF(argv[i], atoi(argv[i+1]));
  }
  return 0;
}


void SDIF2DF(char *filename, sdif_int32 streamID) {
  int goodFrameCount = 0;
  int frameCount;
  SDIF_FrameHeader fh;
  SDIF_MatrixHeader	mh;
  FILE *f;
    SDIFresult r;


  r = SDIF_OpenRead(filename, &f);

  if (r) {
    fprintf(stderr, "Couldn't open %s: %s\n", filename, SDIF_GetErrorString(r));
    return;
  }
  // printf("** Opened for reading\n");

  while (!(r = SDIF_ReadFrameHeader(&fh, f))) {
    int i;

    frameCount++;

    //    printf("** read frame %d\n", frameCount);

    
    if (fh.size < 16) {
      fprintf(stderr, "%s: Frame size count %d too small for frame header\n",
	      filename, fh.size);
      goto close;
    }

    if ((fh.size & 7) != 0) {
      fprintf(stderr, "%s: Frame size count %d is not a multiple of 8\n",
	      filename, fh.size);
      goto close;
    }

    if (fh.streamID != streamID) {
      r = SDIF_SkipFrame(&fh, f);
      if (r) {
	fprintf(stderr, "Problem skipping frame: %s\n", SDIF_GetErrorString(r));
      }
      continue;
    }

    for (i = 0; i < fh.matrixCount; ++i) {
      int j;

      if (r = SDIF_ReadMatrixHeader(&mh,f)) {
	fprintf(stderr, "Problem reading matrix header: %s\n", SDIF_GetErrorString(r));
	goto close;
      } 


      if (!(SDIF_Char4Eq(mh.matrixType, "1TRC") || 
	  SDIF_Char4Eq(mh.matrixType, "1HRM")))  {
	  r = SDIF_SkipMatrix(&mh, f);
	  if (r) {
	    fprintf(stderr, "Problem skipping matrix: %s\n", SDIF_GetErrorString(r));
	  }
      } else {

	if (mh.columnCount != 4) {
	    fprintf(stderr, "Expected 4 columns, got %d (time %f, matrix %c%c%c%c\n",
		    mh.columnCount, fh.time,
		    mh.matrixType[0],mh.matrixType[1],mh.matrixType[2],mh.matrixType[3]);
	}

	/* Write frame header */
	printf("%d %f\n", mh.rowCount, fh.time);

	/* Write sinusoids */
	for (j = 0; j < mh.rowCount; ++j) {

	    switch (mh.matrixDataType) {
		case SDIF_FLOAT32 : {
		    sdif_float32 index32, freq32, amp32, phase32;

		    SDIF_Read4(&index32,1,f);
		    SDIF_Read4(&freq32,1,f);
		    SDIF_Read4(&amp32,1,f);
		    SDIF_Read4(&phase32,1,f);

		    printf("%d %f %f %f\n",
			   (int) index32, freq32, amp32, phase32);
		}
		break;


		case SDIF_FLOAT64 :
		    fprintf (stderr, "can't handle FLOAT64 yet.\n");

		default:
		fprintf (stderr, "%s Unexpected data type %x\n",
			 filename, mh.matrixDataType);
		goto close;
	    }
	}
      }
      printf("\n");

      if ((mh.rowCount * mh.columnCount) & 0x1) {
	  sdif_float32 pad;
	  SDIF_Read4(&pad,1,f);
      }

      goodFrameCount++;

    }
  }
	

  if (r != ESDIF_END_OF_DATA) {
    fprintf(stderr, "%s: Bad frame header!\n", filename);
    goto close;
  }

close:
  SDIF_CloseRead(f);

  if (goodFrameCount == 0) {
    fprintf(stderr, "Warning: no frames found in %s streamID %d\n",
	    filename, streamID);
      }
}
