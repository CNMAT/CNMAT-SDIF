
#include <stdio.h>
#include <string.h>
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

 Amar Chaudhary, 1/13/98

*/


int main (int argc, char *argv[]) {
  int	i,j;
  FILE	*infile,*outfile;
  float	timeval,pitchval;
  float confidence = 1.0f;
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
    strcpy(extension,".sdif");
    
    if ((outfile = OpenSDIFWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s\n",outfname);
      fclose(infile);
      return -1;
    }
    
    Copy4Bytes(head.frameType,"1FQ0");
    head.streamID = GenUniqueSDIFFrameID();
    head.matrixCount = 1;

        
    while (fscanf(infile,"%f %f",&timeval,&pitchval) == 2) {
	head.time = timeval;
	matrixHeader.rowCount = 1;
	matrixHeader.columnCount = 2;
	Copy4Bytes(matrixHeader.matrixType,"1FQ0");
	matrixHeader.matrixDataType = SDIF_FLOAT32;
	head.size = 16 + 16 + 2 * 4 ; 
	WriteSDIFFrameHeader(&head,outfile);
	WriteMatrixHeader(&matrixHeader,outfile);
	write4(&pitchval,1,outfile);
	write4(&confidence,1,outfile);
    }

    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
