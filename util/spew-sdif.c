/* 

Copyright (c) 1997,1998,1999,2000.  The Regents of the University of California
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

 spew-sdif.c

 Utility to print contents of SDIF files to stdout

 Matt Wright, 1/27/97

 1/12/98 - Edited by Amar Chaudhary to conform to revised SDIF spec
 9/15/98 - Now handles 4-byte padding properly (Amar)
 12/16/99: minor tweaks for less compiler warnings, handling printing
           of strings' null termination (Matt)
 3/30/2000: At IRCAM, fix for zero-matrix frames (Matt)
 6/1/2000: Support 16 bit ints
 6/5/2000: Fixed null-termination check bug
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
      fprintf(stderr, "spew-sdif: more than %d unique IDs---too many to remember!\n",
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
	 sawTooMany ? "AT LEAST " : "", numIdsSeen);
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
  FILE *f;
    SDIFresult r;


  r = SDIF_OpenRead(filename, &f);

  if (r) {
    printf("Couldn't open %s: %s\n", filename, SDIF_GetErrorString(r));
    return;
  }


  ForgetIDs();
  frameCount = 0;
  while (!(r = SDIF_ReadFrameHeader(&fh, f))) {
    int i;

    frameCount++;

    printf("Frame %d: Type %c%c%c%c, size %d, time %f, ID %ld, %d matrices\n",
	   frameCount, fh.frameType[0], fh.frameType[1], fh.frameType[2],
	   fh.frameType[3], fh.size, fh.time, fh.streamID, fh.matrixCount);

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

    if (IsNewId(fh.streamID)) {
      printf("---First frame with stream ID %ld\n", fh.streamID);
    }

    if (fh.matrixCount == 0) {
      fprintf(stderr, "%s: Frame with zero matrices!!  (skipping)\n", filename);
      r = SDIF_SkipFrame(&fh, f);
      if (r) {
	fprintf(stderr, "Problem skipping frame: %s\n", SDIF_GetErrorString(r));
      }
    }


    for (i = 0; i < fh.matrixCount; ++i) {
      int j,k;

      if (r = SDIF_ReadMatrixHeader(&mh,f)) {
	printf("Problem reading matrix header: %s\n", SDIF_GetErrorString(r));
	goto close;
      } 

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

      case SDIF_INT16 :
	for (j = 0; j < mh.rowCount; ++j) {
	  for (k = 0; k < mh.columnCount; ++k) {
	    sdif_int16 val16;
	    SDIF_Read2(&val16, 1, f);
	    printf ("\t%d",val16);
	  }
	  puts ("");
	}

	{
	    sdif_int16 pad[3];
	    SDIF_Read2(&pad, SDIF_PaddingRequired(&mh), f);
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

	    /* Column-by-column, see if there are any multibyte characters, 
	       or just ASCII */

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
		    for (j = 0; j < mh.rowCount-1; ++j) {
			int c = data[j*mh.columnCount + k];
			if (c == '\0') {
			    printf("\"\n [next string]\"");
			} else {
			    printf("%c", c);
			}
		    }
		    printf("\"\n");
		    if (data[j*mh.columnCount + k] != '\0') {
			printf("Warning: This UTF-8 column is not properly null-terminated\n");
		    }

#ifdef PRINT_STRINGS_IN_HEX_TOO
		    printf(" Hex: ");
		    for (j = 0; j < mh.rowCount; ++j) {
			printf("%x ", data[j*mh.columnCount + k]);
		    }
		    printf("\n");
#endif
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
	fprintf (stderr, "%s Unrecognized data type %x\n",
		 filename, mh.matrixDataType);
	goto close;
      }
    }
  }

  if (r != ESDIF_END_OF_DATA) {
    fprintf(stderr, "%s: Bad frame header!\n", filename);
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
    if (t == SDIF_INT16) return "SDIF_INT16";
    if (t == SDIF_INT32) return "SDIF_INT32";
    if (t == SDIF_UINT32) return "SDIF_UINT32";
    if (t == SDIF_UTF8) return "SDIF_UTF8";
    if (t == SDIF_BYTE) return "SDIF_BYTE";
    if (t == SDIF_NO_TYPE) return "SDIF_NO_TYPE";
    sprintf(unrecognized, "Unrecognized MatrixDataType %x", t);
    return unrecognized;
}
