/*
 * Copyright(c) 1997,1998 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 spew-sdif.c

 Utility to print contents of SDIF files to stdout

 Matt Wright, 1/27/97

 1/12/98 - Edited by Amar Chaudhary to conform to revised SDIF spec
 9/15/98 - Now handles 4-byte padding properly (Amar)
*/

#include <stdio.h>
#include "sdif.h"
#include "string.h"

typedef int Boolean;
#define FALSE 0
#define TRUE 1


void SpewSDIF(char *filename);

int main(int argc, char *argv[]) {
  int i;

  /* Parse args */
  for (i = 1; i < argc; i++) {
    SpewSDIF(argv[i]);
  }
}

/* A trivial data structure for keeping track of ID numbers in an SDIF file */

#define MAX_IDS_TO_REMEMBER 1000
static int32 idsSeen[MAX_IDS_TO_REMEMBER];
static int numIdsSeen;
static Boolean sawTooMany = FALSE;

void ForgetIDs(void) {
  numIdsSeen = 0;
  sawTooMany = FALSE;
}

Boolean IsNewId(int32 id) {
  int i;
  for (i = 0; i < numIdsSeen; i++) {
    if (idsSeen[i] == id) return FALSE;
  }

  if (numIdsSeen == MAX_IDS_TO_REMEMBER) {
    if (!sawTooMany) {
      fprintf(stderr, "*** Saw more than %d unique IDs---too many to remember!\n",
	      MAX_IDS_TO_REMEMBER);
      sawTooMany = TRUE;
    }
    return TRUE;
  }
  idsSeen[numIdsSeen++] = id;
  return TRUE;
}

void PrintAllIDs(void) {
  int i;
  printf("\n%s%d Unique IDs found in the file:\n",
	 sawTooMany ? "*** AT LEAST " : "", numIdsSeen);
  for (i = 0; i < numIdsSeen; i++) {
    printf("%ld ", idsSeen[i]);
    if (i % 7 == 6) printf("\n");
  }
  printf("\n");
}

void SpewSDIF(char *filename) {
  int size, frameCount;
  struct SDIFFrameHeader fh;
  MatrixHeader	mh;
  FILE *f = OpenSDIFRead(filename);

  if (f == NULL) {
    printf("Couldn't open %s\n", filename);
    return;
  }


  ForgetIDs();
  frameCount = 0;
  while (ReadSDIFFrameHeader(&fh, f) == 1) {
    int i;

    frameCount++;

    printf("Frame %d: Type %c%c%c%c, size %d, time %f, ID %ld, %d matrices\n",
	   frameCount, fh.frameType[0], fh.frameType[1], fh.frameType[2],
	   fh.frameType[3], fh.size, fh.time, fh.streamID, fh.matrixCount);

    if (fh.size < 16) {
      fprintf(stderr, "*** Frame size count %d too small for frame header\n",
	      fh.size);
      goto close;
    }

    if ((fh.size & 7) != 0) {
      fprintf(stderr, "*** Frame size count %d is not a multiple of 8\n",
	      fh.size);
      goto close;
    }

    if (IsNewId(fh.streamID)) {
      printf("---First frame with stream ID %ld\n", fh.streamID);
    }

    for (i = 0; i < fh.matrixCount; ++i) {
      int j,k;
      float32 val32;
      float64 val64;

      ReadMatrixHeader(&mh,f);
      printf ("   Matrix %d: Type %c%c%c%c, DataType %ld, %ld Rows, %ld Columns\n",
	      i,mh.matrixType[0],mh.matrixType[1],mh.matrixType[2],mh.matrixType[3],
	      mh.matrixDataType,mh.rowCount,mh.columnCount);

      switch (mh.matrixDataType) {
      case SDIF_FLOAT32 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    read4(&val32,1,f);
	    printf ("\t%f",val32);
	  }
	  puts ("");
	}
	if ((mh.rowCount * mh.columnCount) & 0x1) {
	    float pad;
	    read4(&pad,1,f);
	}
	break;
      case SDIF_FLOAT64 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    read8(&val64,1,f);
	    printf ("\t%f",val64);
	  }
	  puts ("");
	}
	break;
      default:
	fprintf (stderr, "*** Unrecognized data type %d\n",mh.matrixDataType);
	goto close;
      }
    }

  }

  if (!feof(f)) {
    fprintf(stderr, "*** Bad frame header!\n");
    goto close;
  }

  PrintAllIDs();

close:
  CloseSDIFRead(f);
}
