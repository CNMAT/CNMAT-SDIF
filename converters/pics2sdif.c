
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
  struct SDIFFrameHeader head;
  MatrixHeader matrixHeader;

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
    
    if ((outfile = OpenSDIFWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s\n",outfname);
      fclose(infile);
      return -1;
    }
    
    Copy4Bytes(head.frameType,"1PIC");
    head.streamID = GenUniqueSDIFFrameID();
    head.matrixCount = 1;

        
    while (fscanf(infile,"%d %d",&sampleIndex,&numPics) == 2) {
	head.time = (float)sampleIndex / (float)srate;
	matrixHeader.rowCount = numPics;
	matrixHeader.columnCount = 4;
	Copy4Bytes(matrixHeader.matrixType,"1PIC");
	matrixHeader.matrixDataType = SDIF_FLOAT32;
	head.size = 16 + 16 + 4 * 4 * numPics ; 
	WriteSDIFFrameHeader(&head,outfile);
	WriteMatrixHeader(&matrixHeader,outfile);
	for (j = 0; j < numPics; ++j) {
	    fscanf (infile,"%d %f %f %f %f",&picIndex,&freq,&amp,&confidence,&phase);
	    write4(&amp,1,outfile);
	    write4(&freq,1,outfile);
	    write4(&phase,1,outfile);
	    write4(&confidence,1,outfile);
	}
    }
    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
