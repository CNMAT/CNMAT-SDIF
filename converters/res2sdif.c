
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

*/


RowOf1RES res[1024];

int main (int argc, char *argv[]) {
  int	i,j;
  FILE	*infile,*outfile;
  float	amp,frequency,bandwidth;
  char	*extension, outfname[1024];
  struct SDIFFrameHeader head;
  MatrixHeader matrixHeader;

  for (i = 1; i < argc; ++i){
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"file not found: %s\n",argv[i]);
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    extension = strrchr(outfname,'.');
    strcpy(extension,".res.sdif");
    
    if ((outfile = OpenSDIFWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s\n",outfname);
      fclose(infile);
      return -1;
    }
    
    Copy4Bytes(head.frameType,"1RES");
    head.streamID = GenUniqueSDIFFrameID();
    head.matrixCount = 1;
    head.time = 0.0;
    
    j = 0;
    while (fscanf(infile,"%f %f %f",&frequency,&amp,&bandwidth) == 3) {
      res[j].freq = frequency;
      res[j].amp = pow(10.0,amp * 0.05);
      res[j].bandwidth = M_PI * bandwidth;
      res[j].phase = 0.0f;
      ++j;
    }
    
    if (j > 0) {
      matrixHeader.rowCount = j;
      matrixHeader.columnCount = 4;
      Copy4Bytes(matrixHeader.matrixType,"1RES");
      matrixHeader.matrixDataType = SDIF_FLOAT32;

      head.size = SizeOf1RESFrame(j);
      WriteSDIFFrameHeader(&head,outfile);
      WriteMatrixHeader(&matrixHeader,outfile);
      write4(res,j*4,outfile);
    }
    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
