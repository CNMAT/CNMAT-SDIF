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

Written by Amar Chaudhary, The Center for New Music and Audio Technologies,
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

 pics2sdif.c

 Converts spectral peak (.pics) files to sdif
 Special thanks to the genius who decided that .pics files would measure
 time in samples instead of seconds!

 Amar Chaudhary, 11/2/98
 Updated by Matt Wright, 10/12/99, for new SDIF library

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdif.h"



int main (int argc, char *argv[]) {
  SDIFresult r;
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
    
    if (r = SDIF_OpenWrite(outfname, &outfile)) {
      fprintf (stderr, "Error creating %s: %s\n",outfname,
	       SDIF_GetErrorString(r));
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
