/* 

Copyright (c) 1997,1998,1999.  The Regents of the University of California
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

 dotformat2sdif.c

 Utility to convert .format and .fmt files into sdif files

 Matt Wright, 1/24/97
 Modified 1/13/98 by Amar Chaudhary for new SDIF format
 Modified 10/12/99 for new SDIF library
*/


#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "sdif.h"
#include "sdif-types.h"
#include "readformat.h"

#define streq(s1, s2) (strcmp((s1), (s2)) == 0)

static char *usage = 
"Usage:  %s [files and options...]\n"
"  Each .format file or .fmt file is converted to SDIF\n"
"  Options:\n"
"     -delete Delete .format and .fmt files after successful conversion\n";

static struct {
  Boolean delete;
} settings;


Boolean ConvertFormat(char *filename, formatType t);

#define MAX_NUM_FILES 1000

int main(int argc, char *argv[]) {
  int i, numFiles;
  char *files[MAX_NUM_FILES];
  formatType types[MAX_NUM_FILES];
  char *extension;

  /* Defaults */
  settings.delete = FALSE;
  numFiles = 0;

  /* Parse args */
  for (i = 1; i < argc; i++) {
    extension = strrchr(argv[i], '.');

    if (extension != NULL && streq(extension, ".fmt")) {
      types[numFiles] = BINARY;
      files[numFiles] = argv[i];
      numFiles++;
    } else if (extension != NULL && streq(extension, ".format")) {
      types[numFiles] = ASCII;
      files[numFiles] = argv[i];
      numFiles++;
    } else if (streq(argv[i], "-delete")) {
      settings.delete = TRUE;
    } else {
      fprintf(stderr, usage, argv[0]);
      exit(-1);
    }
  }

  printf("Converting %d files...\n", numFiles);
  if (settings.delete) {
    printf("Deleting old files once converted.\n");
  }

  for (i = 0; i < numFiles; i++) {
    if (ConvertFormat(files[i], types[i]) && settings.delete) {
	unlink(files[i]);
    }
  }
  return 0;
}


Boolean ConvertFormat(char *filename, formatType t) {
  SDIFresult r;
  Boolean Finished = FALSE;
  eitherFormat in;
  FILE *out;
  char outfilename[1000], *extension;
  float time;
  int numTracks, i;
  SDIF_FrameHeader head;
  SDIF_MatrixHeader SDIF_MatrixHeader;
  SDIF_RowOf1TRC trackData;

  if ((in = OpenEitherFormat(filename, t)) == NULL) return FALSE;

  strcpy(outfilename, filename);
  extension = strrchr(outfilename, '.');
  strcpy(extension, ".sdif");

  r = SDIF_OpenWrite(outfilename, &out);
  if (r) {
    fprintf(stderr, "Couldn't open \"%s\" for writing; skipping.\n",
	    outfilename);
    CloseEitherFormat(in);
    return FALSE;
  }

  SDIF_Copy4Bytes(head.frameType, "1TRC");
  head.streamID = SDIF_UniqueStreamID();

  while(ReadFrameHeader(in,&numTracks, &time) != EOF) {
    head.time = (sdif_float64) time;
    head.size = SizeOf1TRCFrame(numTracks);
    head.matrixCount = 1;

    if (r = SDIF_WriteFrameHeader(&head, out)) {
      fprintf (stderr,"Error writing frame header: %s\n",
	       SDIF_GetErrorString(r));
      goto writeerror;
    }


    SDIF_Copy4Bytes(SDIF_MatrixHeader.matrixType, "1TRC");
    SDIF_MatrixHeader.matrixDataType = SDIF_FLOAT32;
    SDIF_MatrixHeader.rowCount = numTracks;
    SDIF_MatrixHeader.columnCount = 4;

    if (r = SDIF_WriteMatrixHeader(&SDIF_MatrixHeader, out)) {
      fprintf (stderr,"Error writing matrix: %s\n", 
	       SDIF_GetErrorString(r));
      goto writeerror;
    }

    for (i = 0; i < numTracks; i++) {
      ReadTrack(in, &trackData.index, &trackData.freq, &trackData.phase,
		&trackData.amp);

      trackData.phase = WrapPhase32(trackData.phase);
      if (trackData.amp < 0.f) {
	trackData.amp = - trackData.amp;
	trackData.phase = - trackData.phase;
      }


      if (r = SDIF_WriteRowOf1TRC(&trackData, out)) {
	fprintf (stderr,"Error writing 1TRC row: %s\n",
		 SDIF_GetErrorString(r));
	goto writeerror;
      }
    }
  }

  Finished = TRUE;


writeerror:
  if (!Finished) {
    fprintf(stderr, "Write error creating SDIF file.  :-(\n");
  }

  CloseEitherFormat(in);
  SDIF_CloseWrite(out);
  return Finished;
}
