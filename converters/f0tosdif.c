/*
Copyright (c) 1998.  The Regents of the University of California (Regents).
All Rights Reserved.

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
*/

/*
 f0tosdif.c

 converts f0 files (rows of frequency estimate, confidence) to SDIF

 Amar Chaudhary, 1/13/98
 updated 10/12/99 by Matt Wright
 
*/

#include <stdio.h>
#include <string.h>
#include "sdif.h"


int main (int argc, char *argv[]) {
  SDIFresult r;
  int	i;
  FILE	*infile,*outfile;
  float	timeval,pitchval, prevtime;
  float confidence = 1.0f;
  char	outfname[1024];
  SDIF_FrameHeader head;
  SDIF_MatrixHeader SDIF_MatrixHeader;

  for (i = 1; i < argc; ++i){
    infile = fopen(argv[i],"rt");
    if (infile == NULL) {
      fprintf (stderr,"file not found: %s\n",argv[i]);
      return -1;
    }
    
    strcpy(outfname,argv[i]);
    strcat(outfname, ".sdif");
    
    if (r = SDIF_OpenWrite(outfname, &outfile)) {
      fprintf (stderr, "Error creating %s: %s\n",outfname,
	       SDIF_GetErrorString(r));
      fclose(infile);
      return -1;
    }
    
    SDIF_Copy4Bytes(head.frameType,"1FQ0");
    head.streamID = SDIF_UniqueStreamID();
    head.matrixCount = 1;


    prevtime = -9999999.0f;
        
    while (fscanf(infile,"%f %f",&timeval,&pitchval) == 2) {
	if (pitchval < 0) {
	    fprintf (stderr, "Error in %s: pitch %f is no good.\n",
		     argv[i], pitchval);
	    return -1;
	}
	if (timeval < prevtime) {
	    fprintf (stderr, "Error in %s: time tag %f followed by time tag %f\n",
		     argv[i], prevtime, timeval);
	    return -1;
	}
	prevtime = timeval;

	head.time = timeval;
	SDIF_MatrixHeader.rowCount = 1;
	SDIF_MatrixHeader.columnCount = 2;
	SDIF_Copy4Bytes(SDIF_MatrixHeader.matrixType,"1FQ0");
	SDIF_MatrixHeader.matrixDataType = SDIF_FLOAT32;
	head.size = 16 + 16 + 2 * 4 ; 
	SDIF_WriteFrameHeader(&head,outfile);
	SDIF_WriteMatrixHeader(&SDIF_MatrixHeader,outfile);
	SDIF_Write4(&pitchval,1,outfile);
	if (SDIF_Write4(&confidence,1,outfile)) {
	    fprintf (stderr, "Error writing %s: %s\n",
		     outfname, SDIF_GetErrorString(r));
	    return -1;
	}
    }

    
    fclose(infile);
    fclose(outfile);
  }

  return 0;
}
