/*
 * Copyright(c) 1997, 1998 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 dotformat2sdif.c

 Utility to convert .format and .fmt files into sdif files

 Matt Wright, 1/24/97
 Modified 1/13/98 by Amar Chaudhary for new SDIF format
*/


xxx must wrap phase


#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "sdif.h"
#include "readformat.h"

typedef int Boolean;
#define FALSE 0
#define TRUE 1

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

void main(int argc, char *argv[]) {
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
}


Boolean ConvertFormat(char *filename, formatType t) {
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

  out = SDIF_OpenWrite(outfilename);
  if (out == NULL) {
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

    if (SDIF_WriteFrameHeader(&head, out) != 1) {
      fprintf (stderr,"Error writing frame header\n");
      goto writeerror;
    }


    SDIF_Copy4Bytes(SDIF_MatrixHeader.matrixType, "1TRC");
    SDIF_MatrixHeader.matrixDataType = SDIF_FLOAT32;
    SDIF_MatrixHeader.rowCount = numTracks;
    SDIF_MatrixHeader.columnCount = 4;

    if (SDIF_WriteMatrixHeader(&SDIF_MatrixHeader, out) != 1) {
      fprintf (stderr,"Error writing matrix\n");
      goto writeerror;
    }

    for (i = 0; i < numTracks; i++) {
      ReadTrack(in, &trackData.index, &trackData.freq, &trackData.phase,
		&trackData.amp);

      if (SDIF_WriteRowOf1TRC(&trackData, out) != 1) {
	fprintf (stderr,"Error writing 1TRC row\n");
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
