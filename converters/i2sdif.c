#if 0

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
*/

 i2sdif.c

 Converts IRCAM FFT result (.i) (.pics) files to sdif

 Amar Chaudhary, 11/2/98
 Updated 10/13/99 by Matt Wright

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdif.h"

int main (int argc, char *argv[]) {
  int	i,firstfile;
  int   numbins,srate;
  FILE	*infile,*outfile;
  float	real,cplx,framesize,resolution,framestep;
  char	*extension, outfname[1024];
  SDIF_FrameHeader head;
  SDIF_MatrixHeader SDIF_MatrixHeader,stiHeader;


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
    
    if ((outfile = SDIF_OpenWrite(outfname)) == NULL) {
      fprintf (stderr, "Error creating %s\n",outfname);
      fclose(infile);
      return -1;
    }
    
    SDIF_Copy4Bytes(head.frameType,"1STF");
    head.streamID = SDIF_UniqueStreamID();
    head.matrixCount = 2;
    head.size = 16 + 16 + 8 + 16 + 8 * numbins;
    head.time = framesize * 0.5;

    stiHeader.rowCount = 1;
    stiHeader.columnCount = 2;
    stiHeader.matrixDataType = SDIF_FLOAT32;
    SDIF_Copy4Bytes(stiHeader.matrixType,"1STI");

    SDIF_MatrixHeader.rowCount = numbins;
    SDIF_MatrixHeader.columnCount = 2;
    SDIF_MatrixHeader.matrixDataType = SDIF_FLOAT32;
    SDIF_Copy4Bytes(SDIF_MatrixHeader.matrixType,"1STF");

    while( !feof(infile)) {
	int i;
	
	SDIF_WriteFrameHeader(&head,outfile);
	SDIF_WriteMatrixHeader(&stiHeader,outfile);
	fwrite(&resolution,sizeof(float),1,outfile);
	fwrite(&framesize,sizeof(float),1,outfile);

	SDIF_WriteMatrixHeader(&SDIF_MatrixHeader,outfile);
	
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
#endif
