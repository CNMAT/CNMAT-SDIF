
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
  int	i,j,firstfile;
  int   numbins,srate;
  FILE	*infile,*outfile;
  float	real,cplx,framesize,resolution,framestep;
  char	*extension, outfname[1024];
  struct SDIFFrameHeader head;
  MatrixHeader matrixHeader,stiHeader;


  srate = 44100;
  numbins = 1024;
  framesize = (float)numbins / (float)srate;
  framestep = 0.01;
  firstfile = 1;

  for (i = 1; i < argc; ++i) {
      if (!strcmp(argv[i],"-srate")) {
	  ++i;
	  srate = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-step")) {
	  ++i;
	  framestep = atof(argv[i]);
      } else if (!strcmp(argv[i],"-size")) {
	  ++i;
	  framesize = atof(argv[i]);
      } else if (!strcmp(argv[i],"-bins")) {
	  ++i;
	  numbins = atoi(argv[i]);
      } else {
	  firstfile = i;
	  break;
      }

  }



  resolution = (float) srate / (float) numbins;

  printf ("Conversion sample rate is %d Hz\n",srate);
  printf ("Frame size is %f seconds\n",framesize);
  printf ("Frame stepping is %f seconds\n",framestep);
  printf ("%d FFT bins\n",numbins);
  printf ("Bin resolution is %f Hz\n",resolution);

  for (i = firstfile; i < argc; ++i){
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"file not found: %s\n",argv[i]);
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    extension = strrchr(outfname,'.');
    strcpy(extension,".i.sdif");
    
    if ((outfile = OpenSDIFWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s\n",outfname);
      fclose(infile);
      return -1;
    }
    
    Copy4Bytes(head.frameType,"1STF");
    head.streamID = GenUniqueSDIFFrameID();
    head.matrixCount = 2;
    head.size = 16 + 16 + 8 + 16 + 8 * numbins;
    head.time = framesize * 0.5;

    stiHeader.rowCount = 1;
    stiHeader.columnCount = 2;
    stiHeader.matrixDataType = SDIF_FLOAT32;
    Copy4Bytes(stiHeader.matrixType,"1STI");

    matrixHeader.rowCount = numbins;
    matrixHeader.columnCount = 2;
    matrixHeader.matrixDataType = SDIF_FLOAT32;
    Copy4Bytes(matrixHeader.matrixType,"1STF");

    while( !feof(infile)) {
	int i;
	
	WriteSDIFFrameHeader(&head,outfile);
	WriteMatrixHeader(&stiHeader,outfile);
	fwrite(&resolution,sizeof(float),1,outfile);
	fwrite(&framesize,sizeof(float),1,outfile);

	WriteMatrixHeader(&matrixHeader,outfile);
	
	for (i = 0; i < numbins; ++i) {
	    fread (&real,sizeof(float),1,infile);
	    fread (&cplx,sizeof(float),1,infile);
	    fwrite(&real,sizeof(float),1,outfile);
	    fwrite(&cplx,sizeof(float),1,outfile);
	}
	head.time += framestep;
    }
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
