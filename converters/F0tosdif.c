
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

 F02sdif.c

 converts "Capital F-zero" files (pitch,loudness,confidence) to SDIF

 Amar Chaudhary, 1/13/98

*/


int main (int argc, char *argv[]) {
  int	i,j;
  FILE	*infile,*outfile;
  float	timeval,pitchval,confidence,amp;
  char	*extension, outfname[1024];
  struct SDIFFrameHeader head;
  MatrixHeader matrixHeader;
  float pad = 0.0f;

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

        
    while (fscanf(infile,"%f %f %f %f",&timeval,&pitchval,&amp,&confidence) == 4) {
	head.time = timeval;
	matrixHeader.rowCount = 1;
	matrixHeader.columnCount = 3;
	Copy4Bytes(matrixHeader.matrixType,"1FQ0");
	matrixHeader.matrixDataType = SDIF_FLOAT32;
	head.size = 16 + 16 + 3 * 4 + 4; 
	WriteSDIFFrameHeader(&head,outfile);
	WriteMatrixHeader(&matrixHeader,outfile);
	write4(&pitchval,1,outfile);
	write4(&confidence,1,outfile);
	amp /= 32768.0f;
	write4(&amp,1,outfile);
	/* write out 4-bytes of padding */
	write4(&pad,1,outfile);
    }

    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
