#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

 pics2sdif.c

 Converts spectral peak (.pics) files to sdif
 Special thanks to the genius who decided that .pics files would measure
 time in samples instead of seconds!

 Amar Chaudhary, 11/2/98

*/


int main (int argc, char *argv[]) {
  int	i,j,firstfile,srate;
  int   sampleIndex,numPics,picIndex;
  FILE	*infile,*outfile;
  float	freq,amp,confidence,phase;
  char	*extension, outfname[1024];
  SDIF_FrameHeader head;
  SDIF_MatrixHeader SDIF_MatrixHeader;

  if (argc > 1 && atoi(argv[1]) > 0) {
      srate = atoi(argv[1]);
      firstfile = 2;
  } else {
      srate = 44100;
      firstfile = 1;
  }

  printf ("Conversion sample rate is %d Hz\n",srate);

  for (i = firstfile; i < argc; ++i){
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"file not found: %s\n",argv[i]);
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    extension = strrchr(outfname,'.');
    strcpy(extension,".pics.sdif");
    
    if ((outfile = SDIF_OpenWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s\n",outfname);
      fclose(infile);
      return -1;
    }
    
    SDIF_Copy4Bytes(head.frameType,"1PIC");
    head.streamID = SDIF_UniqueStreamID();
    head.matrixCount = 1;

        
    while (fscanf(infile,"%d %d",&sampleIndex,&numPics) == 2) {
	head.time = (float)sampleIndex / (float)srate;
	SDIF_MatrixHeader.rowCount = numPics;
	SDIF_MatrixHeader.columnCount = 4;
	SDIF_Copy4Bytes(SDIF_MatrixHeader.matrixType,"1PIC");
	SDIF_MatrixHeader.matrixDataType = SDIF_FLOAT32;
	head.size = 16 + 16 + 4 * 4 * numPics ; 
	SDIF_WriteFrameHeader(&head,outfile);
	SDIF_WriteMatrixHeader(&SDIF_MatrixHeader,outfile);
	for (j = 0; j < numPics; ++j) {
	    fscanf (infile,"%d %f %f %f %f",&picIndex,&freq,&amp,&confidence,&phase);
	    SDIF_Write4(&amp,1,outfile);
	    SDIF_Write4(&freq,1,outfile);
	    SDIF_Write4(&phase,1,outfile);
	    SDIF_Write4(&confidence,1,outfile);
	}
    }
    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
