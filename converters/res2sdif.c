#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sdif.h"


/*
 * Copyright(c) 1997,1998 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 res2sdif.c

 converts resonance text files (rows of frequency, amplitude, bandwidth) to SDIF
 Now supports the latest version of SDIF, as of 7/16/99

 Amar Chaudhary, 7/16/99
 updated by Matt Wright 9/22/99

*/


SDIF_RowOf1RES res[4096];

int main (int argc, char *argv[]) {
  int	i,j;
  FILE	*infile,*outfile;
  float	logamp,frequency,bandwidth;
  char	*extension, outfname[1024];
  SDIF_FrameHeader head;
  SDIF_MatrixHeader mhead;

  for (i = 1; i < argc; ++i){
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"can't open %s: %s\n",argv[i], 
	       SDIF_GetLastErrorString());
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    extension = strrchr(outfname,'.');
    strcpy(extension,".res.sdif");
    
    if ((outfile = SDIF_OpenWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s: %s\n",outfname, 
	       SDIF_GetLastErrorString());
      fclose(infile);
      return -1;
    }
    
    SDIF_Copy4Bytes(head.frameType,"1RES");
    head.streamID = SDIF_UniqueStreamID();
    head.matrixCount = 1;
    head.time = 0.0;
    
    j = 0;
    while (fscanf(infile,"%f %f %f",&frequency,&logamp,&bandwidth) == 3) {
      res[j].freq = frequency;
      res[j].amp = pow(10.0, logamp * 0.05);
      res[j].decayrate = M_PI * bandwidth;
      res[j].phase = 0.0f;
      ++j;
    }
    
    if (j > 0) {
      mhead.rowCount = j;
      mhead.columnCount = 4;
      SDIF_Copy4Bytes(mhead.matrixType,"1RES");
      mhead.matrixDataType = SDIF_FLOAT32;

      head.size = SDIF_SizeOf1RESFrame(j);
      SDIF_WriteFrameHeader(&head,outfile);
      SDIF_WriteMatrixHeader(&mhead, outfile);
      SDIF_Write4(res,j*4,outfile);
    }
    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
