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
#include <stdlib.h>
#include "sdif.h"
#include "string.h"

typedef int Boolean;
#define FALSE 0
#define TRUE 1


void SpewSDIF(char *filename);
char *MDTstring(sdif_int32 t);

void main(int argc, char *argv[]) {
  int i;

  /* Parse args */
  for (i = 1; i < argc; i++) {
    SpewSDIF(argv[i]);
  }
}

/* A trivial data structure for keeping track of ID numbers in an SDIF file */

#define MAX_IDS_TO_REMEMBER 1000
static sdif_int32 idsSeen[MAX_IDS_TO_REMEMBER];
static int numIdsSeen;
static Boolean sawTooMany = FALSE;

void ForgetIDs(void) {
  numIdsSeen = 0;
  sawTooMany = FALSE;
}

Boolean IsNewId(sdif_int32 id) {
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
  int frameCount;
  SDIF_FrameHeader fh;
  SDIF_MatrixHeader	mh;
  FILE *f = SDIF_OpenRead(filename);

  if (f == NULL) {
    printf("Couldn't open %s: %s\n", filename, SDIF_GetLastErrorString());
    return;
  }


  ForgetIDs();
  frameCount = 0;
  while (SDIF_ReadFrameHeader(&fh, f) == 1) {
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

      SDIF_ReadMatrixHeader(&mh,f);
      printf ("   Matrix %d: Type %c%c%c%c, %s, %ld Row%s, %ld Column%s\n",i,
	      mh.matrixType[0],mh.matrixType[1],mh.matrixType[2],mh.matrixType[3],
	      MDTstring(mh.matrixDataType),
	      mh.rowCount, mh.rowCount==1? "" : "s", 
	      mh.columnCount, mh.columnCount==1? "" : "s");



      switch (mh.matrixDataType) {
      case SDIF_FLOAT32 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    sdif_float32 val32;
	    SDIF_Read4(&val32,1,f);
	    printf ("\t%f",val32);
	  }
	  puts ("");
	}
	if ((mh.rowCount * mh.columnCount) & 0x1) {
	    sdif_float32 pad;
	    SDIF_Read4(&pad,1,f);
	}
	break;

      case SDIF_FLOAT64 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    sdif_float64 val64;
	    SDIF_Read8(&val64,1,f);
	    printf ("\t%f",val64);
	  }
	  puts ("");
	}
	break;

      case SDIF_INT32 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    sdif_int32 val32;
	    SDIF_Read4(&val32,1,f);
	    printf ("\t%d",val32);
	  }
	  puts ("");
	}
	if ((mh.rowCount * mh.columnCount) & 0x1) {
	    sdif_int32 pad;
	    SDIF_Read4(&pad,1,f);
	}
	break;

      case SDIF_UINT32 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    sdif_uint32 val32;
	    SDIF_Read4(&val32,1,f);
	    printf ("\t%u",val32);
	  }
	  puts ("");
	}
	if ((mh.rowCount * mh.columnCount) & 0x1) {
	    sdif_int32 pad;
	    SDIF_Read4(&pad,1,f);
	}
	break;

      case SDIF_UTF8: {
	    /* Read the whole thing into memory */
	    int numBytes = SDIF_GetMatrixDataSize(&mh); /* includes padding */
	    char *data = malloc(numBytes);
	    SDIF_Read1(data, numBytes, f);

	    /* Column-by-column, see if it's really UTF8 or just ASCII */

#define IsASCII(c) (!((c) & 128))
	    for (k = 0; k < mh.columnCount; ++k) { 
		int seenNonASCII = 0;
		for (j = 0; j < mh.rowCount; ++j) {  
		    if (!IsASCII(data[j*mh.columnCount + k])) {
			seenNonASCII = 1;
			break;
		    }
		}
		if (seenNonASCII) {
		    printf("\t[Column %d has non-ASCII characters.  "
			   "Sorry; no UTF8 support yet.]\n", k);
		} else {
		    printf("[%d] \"", k);
		    for (j = 0; j < mh.rowCount; ++j) {
			printf("%c", data[j*mh.columnCount + k]);
		    }
		    printf("\"\n");
		}
	    }
	    free(data);
      }	break;

      case SDIF_BYTE:
	for (j = 0; j < mh.rowCount; ++j) {
          for (k = 0; k < mh.columnCount; ++k) {
	    unsigned char val8;
	    SDIF_Read1(&val8, 1, f);
            printf(" %x (%c)\t", val8, val8);
	  }
	  putchar('\n');
	}
	if ((mh.rowCount * mh.columnCount) & 0x7) {
	    unsigned char pad[7];
	    SDIF_Read1(&pad, 8-((mh.rowCount * mh.columnCount) & 0x7), f);
	}
	break;

      default:
	fprintf (stderr, "*** Unrecognized data type %x\n",mh.matrixDataType);
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
  SDIF_CloseRead(f);
}


char unrecognized[50];

char *MDTstring(sdif_int32 t) {
    if (t == SDIF_FLOAT32) return "SDIF_FLOAT32";
    if (t == SDIF_FLOAT64) return "SDIF_FLOAT64";
    if (t == SDIF_INT32) return "SDIF_INT32";
    if (t == SDIF_UINT32) return "SDIF_UINT32";
    if (t == SDIF_UTF8) return "SDIF_UTF8";
    if (t == SDIF_BYTE) return "SDIF_BYTE";
    if (t == SDIF_NO_TYPE) return "SDIF_NO_TYPE";
    sprintf(unrecognized, "Unrecognized MatrixDataType %x", t);
    return unrecognized;
}
